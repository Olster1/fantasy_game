typedef struct {
	int parentId; //the index in array
	char *name;
	Matrix4 toJointSpace;
} EasySkeletonJoint;

typedef struct {
	char *name;

	int jointCount;
	InfiniteAlloc joints;
} EasySkeleton;

typedef struct {
	V4 Q;
	V3 pos;
	float scale;
} EasySkeleton_SQT;

typedef struct {
	EasySkeleton_SQT *jointPoses; //count 
} EasySkeleton_AnimationPose;

typedef struct {	
	char *name;
	float duration;

	int poseCount;
	EasySkeleton_AnimationPose *poseSamples;
} EasySkeleton_AnimationClip;

typedef struct EasySkeleton_AnimationState EasySkeleton_AnimationState;
typedef struct EasySkeleton_AnimationState {	
	EasySkeleton_AnimationClip *clip;
	float tAt; 
	float blendWeight;
	bool loop;

	float playbackSpeed; //can be negative for reverse animations

	EasySkeleton_AnimationState *next; //linked list
} EasySkeleton_AnimationState;

typedef struct {	
	EasySkeleton *skeleton;

	int activeStateCount; //so we can allocate an array to store precomputed samples
	EasySkeleton_AnimationState *states;

	EasySkeleton_AnimationState *freeList;	
} EasySkeleton_Controller;

static inline void easySkeleton_initSkeleton(EasySkeleton *s, char *name) {
	s->joints = initInfinteAlloc(EasySkeletonJoint);
	s->name = name;
}

static inline EasySkeleton_SQT easySkeleton_lerpSQT(EasySkeleton_SQT a, EasySkeleton_SQT b, float t) {
	EasySkeleton_SQT result;

	result.Q = lerpV4(a.Q, t, b.Q);
	result.pos = lerpV3(a.pos, t, b.pos);
	result.scale = lerp(a.scale, t, b.scale);

	return result;
}

static inline Matrix4 easySkeleton_SQT_to_Matrix4(EasySkeleton_SQT in) {
 	Matrix4 result = quaternionToMatrix(easyMath_v4ToQuaternion(in.Q));
	result = Mat4Mult(Matrix4_scale(mat4(), v3(in.scale, in.scale, in.scale)), result);
	result = Mat4Mult(Matrix4_translate(mat4(), in.pos), result);

	return result;
}
typedef struct {
	Matrix4 *palette;
	int count;
} EasySkeleton_MatrixPalette;


static inline EasySkeleton_SQT easySkeleton_getLocalJointSample(EasySkeleton_AnimationClip *clip, float tAt, int jointIndex) {
	float normalizedTime = tAt / clip->duration;

	float samplePosReal = normalizedTime * ((float)clip->poseCount - 1);

	int sampleIndexA = (int)samplePosReal;
	int sampleIndexB = sampleIndexA + 1;

	float blendFactor = samplePosReal - sampleIndexA;

	EasySkeleton_AnimationPose *poseA = &clip->poseSamples[sampleIndexA];
	EasySkeleton_AnimationPose *poseB = &clip->poseSamples[sampleIndexB];

	EasySkeleton_SQT result = easySkeleton_lerpSQT(poseA->jointPoses[jointIndex], poseB->jointPoses[jointIndex], blendFactor);
	return result;
}

static inline EasySkeleton_SQT easySkeleton_addSQT(EasySkeleton_SQT soFar, EasySkeleton_SQT sample, float blendWeight) {
	//linear weighted blend
	soFar.Q = v4_plus(v4_scale(blendWeight, sample.Q), soFar.Q);
	soFar.pos = v3_plus(v3_scale(blendWeight, sample.pos), soFar.pos);
	soFar.scale = blendWeight*sample.scale + soFar.scale;
	return soFar;
}

static inline EasySkeleton_AnimationState *easySkeleton_pushState(EasySkeleton_Controller *controller, EasySkeleton_AnimationClip *clip, bool loop, float playbackSpeed) {
	EasySkeleton_AnimationState *newState = controller->freeList;

	if(newState) {
		newState = pushStruct(&globalLongTermArena, EasySkeleton_AnimationState);
		
	} else {
		//pull off free list
		controller->freeList = newState->next;
	}
	assert(newState);

	//update the number of active states 
	controller->activeStateCount++;

	//add to the linked list
	newState->next = controller->states;
	controller->states = newState;
	//

	//assign the attributes
	newState->clip = clip;
	newState->tAt = 0; 
	newState->blendWeight = 1.0f;
	newState->loop = loop;

	newState->playbackSpeed = playbackSpeed; //can be negative for reverse animations

	return newState;
}

static inline bool easySkeleton_isAnimationFinished(EasySkeleton_AnimationState *state) {
	bool result = state->tAt >= state->clip->duration;
	if(state->playbackSpeed < 0.0f) {
		result = state->tAt < 0.0f;
	} 
	return result;
}

//NOTE: This is the function we call to pass to the vertex shader. The rest of the functions above this are helper functions used in this funciton. 
//NOTE: The idea of a Matrix palette is that each vertex is bound to some joint. The vertex starts in the default model position. Each animation pose
//		has a a list of joints with a local to parent joint space matrix. So you select the joint the vertex is attached to,  
static inline EasySkeleton_MatrixPalette easySkeleton_buildSkinningPalette(EasySkeleton_Controller *contoller, float dt) {
	EasySkeleton *s = contoller->skeleton;

	EasySkeleton_MatrixPalette result;
	result.count = s->jointCount;
	result.palette = pushArray(&globalPerFrameArena, s->jointCount, Matrix4);

	if(contoller->activeStateCount > 0) { //has animations to process

		//build local joint samples for each animation clip playing, so we don't build them multiple times.
		//Say if joint 3 references joint 2, and joint 4 references joint 2. 
		//When we walk joint 3 we would sample joint 2 local SQT, and when we walk joint 4 we would sample joint 2 again. 
		//So we precache the local transforms at each joint, then we can just grab them when ever we need them.
		EasySkeleton_SQT **activeStates = pushArray(&globalPerFrameArena, contoller->activeStateCount, EasySkeleton_SQT *);	
		EasySkeleton_AnimationState *activeState = contoller->states;
		assert(activeState);
		for(int stateIndex = 0; stateIndex < contoller->activeStateCount; ++stateIndex) {
			assert(activeState);

			activeStates[stateIndex] = pushArray(&globalPerFrameArena, s->jointCount, EasySkeleton_SQT);	

			EasySkeleton_SQT *localPoseArray = activeStates[stateIndex]; //derefernce to get another pointer
			for(int jointIndex = 0; jointIndex < s->jointCount; ++jointIndex) {
				localPoseArray[jointIndex] = easySkeleton_getLocalJointSample(activeState->clip, activeState->tAt, jointIndex);
			}
			activeState = activeState->next;
		}
		///



		//walk parent hirearchy to get the globalJointToModel Transform for each joint
		for(int i = 0; i < s->jointCount; i++) {
			Matrix4 skinningMatrix = mat4();
			
			EasySkeletonJoint *joint = getElementFromAlloc(&contoller->skeleton->joints, i, EasySkeletonJoint); 
			assert(joint);

			int jointIndex = i;
			while(joint) {

				EasySkeleton_SQT blendedSQT = {};
				EasySkeleton_AnimationState *state = contoller->states;
				int stateIndex = 0;

				//get the local joint SQT for all then animations playing via a linear weighted blend (n dimensional lerp)
				while(state) {
					assert(stateIndex < contoller->activeStateCount);

					EasySkeleton_SQT *localPoseArray = activeStates[stateIndex];
					EasySkeleton_SQT sample = localPoseArray[jointIndex];

					//blend with the same joint in the other animation clips
					blendedSQT = easySkeleton_addSQT(blendedSQT, sample, state->blendWeight);
					
					stateIndex++;
					state = state->next;
				}

				//for the local joint, turn the SQT transform to a matrix4 transform
				Matrix4 jToM = easySkeleton_SQT_to_Matrix4(blendedSQT);

				//concate local matrix to the children transforms 
				skinningMatrix = Mat4Mult(jToM, skinningMatrix);
				//


				//go to next parent up the bone hierachy 
				if(joint->parentId >= 0) {
					jointIndex = joint->parentId;
					joint = getElementFromAlloc(&contoller->skeleton->joints, joint->parentId, EasySkeletonJoint);
				} else {
					//we've built the localBoneToModel Transform
					joint = 0;
					break;
				}
				//
			}

			//multiple it by the inverse default bind poseition transform, and assign it to out to palette
			result.palette[i] = Mat4Mult(skinningMatrix, joint->toJointSpace); //do our inverse bind position multipled by local to global joint matrix
		}

		//update animation states. Do we want this in a seperate function? Probably
		EasySkeleton_AnimationState *state = contoller->states;
		while(state) {
			state->tAt += state->playbackSpeed*dt;
			if(easySkeleton_isAnimationFinished(state)) { //have this for negative playback speeds

				if(state->loop) {
					//go back to start
					state->tAt = state->tAt - signOf(state->playbackSpeed)*state->clip->duration;
				} else {
					assert(contoller->activeStateCount > 0);
					contoller->activeStateCount--;
					//pull off list
					contoller->states = state->next;
					//

					//put on list
					state->next = contoller->freeList;
					contoller->freeList = state;
					//
				}	
				
			}
			state = state->next;
		}
	} else {
		//set all the matrixes to identity matrixes
		for(int i = 0; i < s->jointCount; i++) {
			result.palette[i] = mat4();
		}
	}

	return result;
}