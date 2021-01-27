#ifndef PHYSICS_TIME_STEP
#define PHYSICS_TIME_STEP 0.002f 
#endif

typedef struct {
	bool collided;
	V2 point;
	V2 normal;
	float distance;
} RayCastInfo;

//TODO: Think more about when the line is parrelel to the edge. 

void easy_phys_updatePosAndVel(V3 *pos, V3 *dP, V3 dPP, float dtValue, float dragFactor) {
	*pos = v3_plus(*pos, v3_plus(v3_scale(sqr(dtValue), dPP),  v3_scale(dtValue, *dP)));
	*dP = v3_plus(*dP, v3_minus(v3_scale(dtValue, dPP), v3_scale(dragFactor, *dP)));
}

//assumes the shape is clockwise
RayCastInfo easy_phys_castRay(V2 startP, V2 ray, V2 *points, int count) {
	DEBUG_TIME_BLOCK()
	isNanErrorV2(startP);
	RayCastInfo result = {};
	if(!(ray.x == 0 && ray.y == 0)) {
		float min_tAt = 0;
		bool isSet = false;
		for(int i = 0; i < count; ++i) {
			V2 pA = points[i];
			isNanErrorV2(pA);
			int bIndex = i + 1;
			if(bIndex == count) {
				bIndex = 0;
			}

			V2 pB = points[bIndex];
			isNanErrorV2(pB);
			V2 endP = v2_plus(startP, ray);
			isNanErrorV2(endP);

			V2 ba = v2_minus(pB, pA);
			isNanErrorV2(ba);

			V2 sa = v2_minus(startP, pA);
			isNanErrorV2(sa);
			V2 ea = v2_minus(endP, pA);
			isNanErrorV2(ea);
			float sideLength = getLength(ba);
			V2 normalBA = normalize_(ba, sideLength);
			isNanErrorV2(normalBA);
			V2 perpBA = perp(normalBA);
			isNanErrorV2(perpBA);
			////
			float endValueX = dotV2(perpBA, ea);
			isNanErrorf(endValueX);
			float startValueX = dotV2(perpBA, sa); 
			isNanErrorf(startValueX);

			float endValueY = dotV2(normalBA, ea);
			isNanErrorf(endValueY);
			float startValueY = dotV2(normalBA, sa); 
			isNanErrorf(startValueY);

			float tAt = inverse_lerp(startValueX, 0, endValueX);
			if(startValueX == endValueX) { //This is when the line is parrellel to the side. 
				if(startValueX != 0) {
					tAt = -1;
				}
			}
			isNanErrorf(tAt);

			if(tAt >= 0 && tAt < 1) {
				float yAt = lerp(startValueY, tAt, endValueY);
				isNanErrorf(yAt);
				if(yAt >= 0 && yAt < sideLength) {
					if(tAt < min_tAt || !isSet) {
						assert(signOf(startValueX) != signOf(endValueX) || (startValueX == 0 && endValueX == 0) || (startValueX == 0.0f || endValueX == 0.0f));
						float xAt = lerp(startValueX, tAt, endValueX); 
						assert(floatEqual_withError(xAt, 0));
						isNanErrorf(tAt);
						isNanErrorf(startValueX);
						isNanErrorf(endValueX);
						isNanErrorf(xAt);
						result.collided = true;
						result.normal = perp(normalBA);
						result.point = v2_plus(v2(xAt, yAt), pA);
						result.distance = getLength(v2_minus(result.point, startP));
						min_tAt = tAt;
					}
				}
			}
		}
	}
	return result;
}

typedef struct {
	V3 hitP;
	float hitT; 
	bool didHit;
} EasyPhysics_RayCastAABB3f_Info;

static inline EasyPhysics_RayCastAABB3f_Info EasyPhysics_CastRayAgainstAABB3f(Matrix4 boxRotation, V3 boxCenter, V3 boxScale, Rect3f bounds, V3 rayDirection, V3 rayPos) {
	DEBUG_TIME_BLOCK()
	EasyPhysics_RayCastAABB3f_Info result = {};

	EasyRay r = {};
	r.origin = v3_minus(rayPos, boxCenter); //offset it from the box center

	//Doesn't have to be normalized direction since we do it in the function
	r.direction = rayDirection;
	r = EasyMath_transformRay(r, mat4_transpose(boxRotation));

	bounds.min = v3_hadamard(boxScale, bounds.min);
	bounds.max = v3_hadamard(boxScale, bounds.max);

	result.didHit = easyMath_rayVsAABB3f(r.origin, r.direction, bounds, &result.hitP, &result.hitT);

	return result;
}


typedef enum {
	EASY_COLLISION_LAYER_WORLD,
	EASY_COLLISION_LAYER_ITEMS,
	EASY_COLLISION_LAYER_ENEMIES,
	EASY_COLLISION_LAYER_PLAYER,
	EASY_COLLISION_LAYER_PLAYER_BULLET,
	EASY_COLLISION_LAYER_ITEM_RIGID,
} EasyCollisionLayer;


static inline bool EasyPhysics_layersCanCollider(EasyCollisionLayer a, EasyCollisionLayer b) {
	bool result = false;
	if(a == b) {

	} else if(a == EASY_COLLISION_LAYER_WORLD || b == EASY_COLLISION_LAYER_WORLD) {
		result = true;
	} else if((a == EASY_COLLISION_LAYER_PLAYER_BULLET || b == EASY_COLLISION_LAYER_PLAYER_BULLET) && (a == EASY_COLLISION_LAYER_ENEMIES || b == EASY_COLLISION_LAYER_ENEMIES)) {
		result = true;
	} else if((a == EASY_COLLISION_LAYER_ITEMS || b == EASY_COLLISION_LAYER_ITEMS) && (a == EASY_COLLISION_LAYER_PLAYER || b == EASY_COLLISION_LAYER_PLAYER)) {
		result = true;
		// easyConsole_addToStream(DEBUG_globalEasyConsole, "ITEM");
	}

	return result;
} 

typedef struct {
	int arrayIndex;
	
	V3 dP;
	V3 accumForce; //perFrame
	V3 accumForceOnce; //Gets cleared after one phsyics click
	V3 accumTorque;
	V3 accumTorqueOnce;

	union {
		struct { //2d
			float inverse_I;
		};
		struct { //3d
			V3 dA;
			// Matrix4 inverse_I;
		};
	};
	
	float inverseWeight;
	float reboundFactor;

	float dragFactor;
	float gravityFactor;

	bool updated;

	u32 isGrounded; //flag where grounded is 1 << 0 & done this frame is 1 << 1 

} EasyRigidBody;

typedef enum {
	EASY_COLLISION_ENTER,
	EASY_COLLISION_STAY,
	EASY_COLLISION_EXIT,
} EasyCollisionType;

typedef struct {
	EasyCollisionType type;
	int objectId; 

	bool hitThisFrame_; //only used internally 
} EasyCollisionInfo;

typedef enum {
	EASY_COLLIDER_CIRCLE, //2d
	EASY_COLLIDER_RECTANGLE, //2d
	EASY_COLLIDER_SPHERE, //3d
	EASY_COLLIDER_CYLINDER, //3d
	EASY_COLLIDER_BOX, //3d
 	EASY_COLLIDER_CAPSULE, //3d
	EASY_COLLIDER_CONVEX_HULL, //3d
} EasyColliderType;

typedef struct {
	int arrayIndex;

	EasyTransform *T;
	EasyRigidBody *rb;

	EasyColliderType type;
	V3 offset;

	bool isTrigger;

	EasyCollisionLayer layer;

	int collisionCount;
	EasyCollisionInfo collisions[256];

	union {
		struct {
			float radius;
		};
		struct {
			float capsuleRadius;
			float capsuleLength;
		};
		struct {
			V3 dim3f;
		};
		struct {
			V2 dim2f;
		};
	};
} EasyCollider;


typedef struct {
	Array_Dynamic colliders;
	Array_Dynamic rigidBodies;
	float physicsTime;
} EasyPhysics_World;


static void EasyPhysics_beginWorld(EasyPhysics_World *world) {
	DEBUG_TIME_BLOCK()
	initArray(&world->colliders, EasyCollider);
	initArray(&world->rigidBodies, EasyRigidBody);
}


static void EasyPhysics_removeCollider(EasyPhysics_World *world, EasyCollider *collider) {
	for (int i = 0; i < world->colliders.count; ++i) {
	    EasyCollider *col = (EasyCollider *)getElement(&world->colliders, i);
	    if(col == collider) {
	    	removeElement_ordered(&world->colliders, i);
	    	break;
	    }
	}
}


static void EasyPhysics_removeRigidBody(EasyPhysics_World *world, EasyRigidBody *body) {
	for (int i = 0; i < world->rigidBodies.count; ++i) {
	    EasyRigidBody *rb = (EasyRigidBody *)getElement(&world->rigidBodies, i);
	    if(rb == body) {
	    	removeElement_ordered(&world->rigidBodies, i);
	    	break;
	    }
	}
}


/*
How to use Collision info:
	
	for(colliders) {
		if(was collision) {
			EasyCollider_addCollisionInfo(colA, hitObjectId_B);
			EasyCollider_addCollisionInfo(colB, hitObjectId_A);
		}
	}
		
	//clean up all collisions (basically checks if still colliding )
	for(colliders) {
		EasyCollider_removeCollisions(collider)
	}
*/

static EasyCollisionInfo *EasyCollider_addCollisionInfo(EasyCollider *col, int hitObjectId) {
	DEBUG_TIME_BLOCK()
	EasyCollisionInfo *result = 0;

	for(int i = 0; i < col->collisionCount; ++i) {
		EasyCollisionInfo *info = col->collisions + i;

		if(info->objectId == hitObjectId) {
			result = info;
			
			if(!result->hitThisFrame_ && result->type == EASY_COLLISION_ENTER) {
				result->type = EASY_COLLISION_STAY; //already in the array, so is a stay type
			}
			

			if (result->type == EASY_COLLISION_EXIT) {
				assert(!result->hitThisFrame_);
				result->type = EASY_COLLISION_ENTER;
			}
			
			break;
		}	
	}

	if(!result) {
		//get a new collision
		assert(col->collisionCount < arrayCount(col->collisions));
		result =  col->collisions + col->collisionCount++;
		result->type = EASY_COLLISION_ENTER;
	}

	assert(result);

	result->hitThisFrame_ = true;
	result->objectId = hitObjectId;

	return result;
}

static void EasyCollider_removeCollisions(EasyCollider *col) {
	DEBUG_TIME_BLOCK()
	for(int i = 0; i < col->collisionCount;) {
		bool increment = true;
		EasyCollisionInfo *info = col->collisions + i;

		if(!info->hitThisFrame_) {
			if(info->type != EASY_COLLISION_EXIT) {
				//NOTE(ollie): set collision to an exit collision
				info->type = EASY_COLLISION_EXIT;
			} else {
				//NOTE(ollie): remove the collision info from the array
				//Take from the end & fill the gap
				col->collisions[i] = col->collisions[--col->collisionCount];
				increment = false;
			}
		}

		if(increment) {
			i++;
		}
	}

///////////////////////************ Empty out all hits this frame*************////////////////////
	for(int i = 0; i < col->collisionCount; i++) {
		EasyCollisionInfo *info = col->collisions + i;
		info->hitThisFrame_ = false;
	}
////////////////////////////////////////////////////////////////////

}

	
static inline EasyCollisionOutput EasyPhysics_SolveRigidBodies(EasyCollider *a_, EasyCollider *b_) {
	DEBUG_TIME_BLOCK()
	EasyCollisionPolygon a = {};
	EasyCollisionPolygon b = {};

	V2 aSize = a_->dim2f;
	V2 bSize = b_->dim2f;

	// char string[512];
	// sprintf(string, "size: %f %f", aSize.x, aSize.y);
	// easyConsole_addToStream(DEBUG_globalEasyConsole, string);


	// sprintf(string, "size: %f %f", bSize.x, bSize.y);
	// easyConsole_addToStream(DEBUG_globalEasyConsole, string);


	
	a.p[0] = v3(-0.5, -0.5, 0);
	a.p[1] = v3(-0.5, 0.5, 0);
	a.p[2] = v3(0.5, 0.5, 0);
	a.p[3] = v3(0.5, -0.5, 0);
	a.count = 4; 

	b.p[0] = v3(-0.5, -0.5, 0);
	b.p[1] = v3(-0.5, 0.5, 0);
	b.p[2] = v3(0.5, 0.5, 0);
	b.p[3] = v3(0.5, -0.5, 0);
	b.count = 4; 

	a.p[0].x *= aSize.x;
	a.p[0].y *= aSize.y;

	a.p[1].x *= aSize.x;
	a.p[1].y *= aSize.y;

	a.p[2].x *= aSize.x;
	a.p[2].y *= aSize.y;

	a.p[3].x *= aSize.x;
	a.p[3].y *= aSize.y;

	b.p[0].x *= bSize.x;
	b.p[0].y *= bSize.y;

	b.p[1].x *= bSize.x;
	b.p[1].y *= bSize.y;

	b.p[2].x *= bSize.x;
	b.p[2].y *= bSize.y;

	b.p[3].x *= bSize.x;
	b.p[3].y *= bSize.y;



	// printf("%f %f %f\n", b_->T->pos.x, b_->T->pos.y, b_->T->pos.z);
	// printf("%f %f %f\n", bT_global.E_[12], bT_global.E_[13], bT_global.E_[14]);

	b.T = easyTransform_getTransform(b_->T);
	a.T = easyTransform_getTransform(a_->T);


	// printf("%f %f %f\n", b.T.E_[12], b.T.E_[13], b.T.E_[14]);
	// printf("-----------------\n");

	EasyCollisionInput input = {};
	input.a = a;
	input.b = b;

	EasyCollisionOutput output = {};
	easyCollision_GetClosestPoint(&input, &output);

	// ouput.pointA; 
	// ouput.pointB; 
	if(output.wasInside) {
		// printf("collision: %f\n", output.distance);
	}
	return output;
}

void EasyPhysics_ResolveCollisions(EasyRigidBody *ent, EasyRigidBody *testEnt, EasyTransform *A, EasyTransform *B, EasyCollisionOutput *output, V3 lastPosA, Quaternion lastQA, V2 *deltaPos) {
	// V2 AP = v2_minus(output->pointA.xy, A->pos.xy);
	// isNanErrorV2(AP);
	// // printf("%f %f\n", output->pointB.x, output->pointB.y);
	// // printf("%f %f\n", testEnt->T->pos.x, testEnt->T->pos.y);
	// V2 BP = v2_minus(output->pointB.xy, B->pos.xy);
	// isNanErrorV2(BP);
	    
	// V2 Velocity_A = v2_plus(ent->dP.xy, v2_scale(ent->dA, perp(AP)));
	// isNanErrorV2(Velocity_A);
	// V2 Velocity_B = v2_plus(testEnt->dP.xy, v2_scale(testEnt->dA, perp(BP)));
	// isNanErrorV2(Velocity_B);

	// V2 RelVelocity = v2_minus(Velocity_A, Velocity_B);
	// isNanErrorV2(RelVelocity);

	// float R = 1.0f; //NOTE(oliver): CoefficientOfRestitution

	// float Inv_BodyA_Mass = ent->inverseWeight;
	// float Inv_BodyB_Mass = testEnt->inverseWeight;
	        
	// float Inv_BodyA_I = ent->inverse_I;
	// float Inv_BodyB_I = testEnt->inverse_I;
	// V2 N = output->normal.xy;
	        
	// float J_Numerator = dotV2(v2_scale(-(1.0f + R), RelVelocity), N);
	// float J_Denominator = dotV2(N, N)*(Inv_BodyA_Mass + Inv_BodyB_Mass) +
	//     (sqr(dotV2(perp(AP), N)) * Inv_BodyA_I) + (sqr(dotV2(perp(BP), N)) * Inv_BodyB_I);

	// float J = J_Numerator / J_Denominator;
	// isNanf(J);

	// V2 Impulse = v2_scale(J, N);
	// isNanErrorV2(Impulse);
	    
	// ent->dP.xy = v2_plus(ent->dP.xy, v2_scale(Inv_BodyA_Mass, Impulse));                
	// isNanErrorV2(ent->dP.xy);
	// testEnt->dP.xy = v2_minus(testEnt->dP.xy, v2_scale(Inv_BodyB_Mass, Impulse));
	// isNanErrorV2(testEnt->dP.xy);
	// ent->dA = ent->dA + (dotV2(perp(AP), Impulse)*Inv_BodyA_I);
	// testEnt->dA = testEnt->dA - (dotV2(perp(BP), Impulse)*Inv_BodyB_I);

	// if(output->wasInside) {
		// assert(false);
		// gjk_v2 a[4] = {0};
		// gjk_v2 b[4] = {0};

		// Gjk_EPA_Info info = gjk_objectsCollide_withEPA(&gjk_a, 4, &gjk_b, 4);

		// a.T.pos.xy = v2_plus(a.T.pos.xy, v2_scale(info.distance, v2(info.normal.x, info.normal.y)));
	// } else 
	{
		
		V2 N = output->normal.xy;

		float max_dP = 1.0f;
		float r0 = 1.0f + ent->reboundFactor;

		ent->dP.xy = v2_minus(ent->dP.xy, v2_scale(r0*dotV2(ent->dP.xy, N), N));

		float r1 = 1.0f + testEnt->reboundFactor;

		testEnt->dP.xy = v2_plus(testEnt->dP.xy, v2_scale(r1*dotV2(testEnt->dP.xy, N), N));


		// char string[512];
		// sprintf(string, "dP 0: %f %f", deltaPos->x, deltaPos->y);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, string);

		*deltaPos = v2_minus(*deltaPos, v2_scale(dotV2(*deltaPos, N), N));

		// sprintf(string, "dP 1: %f %f", deltaPos->x, deltaPos->y);
		// easyConsole_addToStream(DEBUG_globalEasyConsole, string);
		
	}
}

static EasyRigidBody *EasyPhysics_AddRigidBody(EasyPhysics_World *world, float inverseWeight, float inverseIntertia, float dragFactor, float gravityFactor) {
	DEBUG_TIME_BLOCK()
    ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&world->rigidBodies);

    EasyRigidBody *rb = (EasyRigidBody *)arrayInfo.elm;

    // memset(rb, 0, sizeof(EasyRigidBody));
    rb->arrayIndex = arrayInfo.absIndex;

    rb->dP = v3(0, 0, 0);
    rb->inverseWeight = inverseWeight;
    rb->inverse_I = inverseIntertia;
    rb->dA = NULL_VECTOR3;
    rb->accumForce = NULL_VECTOR3;
    rb->accumTorque = NULL_VECTOR3;
    rb->accumForceOnce = NULL_VECTOR3;
    rb->accumTorqueOnce = NULL_VECTOR3;
    rb->dragFactor = dragFactor;
    rb->gravityFactor = gravityFactor;
    rb->isGrounded = 0;
    rb->reboundFactor = 0.0f;
    rb->updated = false;

    return rb;
}


static EasyCollider *EasyPhysics_AddCollider(EasyPhysics_World *world, EasyTransform *T, EasyRigidBody *rb, EasyColliderType type, V3 offset, bool isTrigger, V3 info) {
	DEBUG_TIME_BLOCK()
	ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&world->colliders);

	EasyCollider *col = (EasyCollider *)arrayInfo.elm;
	memset(col, 0, sizeof(EasyCollider));
	
	col->arrayIndex = arrayInfo.absIndex;

	assert(T);
	col->T = T;
	col->rb = rb;

	col->type = type;
	col->offset = offset;

	col->isTrigger = isTrigger;

	col->collisionCount = 0;

	switch (type) {
		case EASY_COLLIDER_SPHERE:
		case EASY_COLLIDER_CIRCLE: {
			col->radius = info.x;
		} break;
		case EASY_COLLIDER_RECTANGLE: {
			col->dim2f = info.xy;
		} break;
		case EASY_COLLIDER_BOX: {
			col->dim3f = info;
		} break;
		default: {
			assert(false);
		}
	}

    return col;
}

typedef struct {
	EasyCollider *hitEnt;
	EasyCollisionOutput outputInfo;
} EasyPhysics_HitBundle;

static V3 EasyPhysics_AddGravity(EasyRigidBody *rb, float scale) {
	V3 gravity = v3(0, scale*-9.81f, 0);

	return gravity;
}

void ProcessPhysics(Array_Dynamic *colliders, Array_Dynamic *rigidBodies, float dt) {
	DEBUG_TIME_BLOCK()
	for (int i = 0; i < rigidBodies->count; ++i)
	{
	    EasyRigidBody *rb = (EasyRigidBody *)getElement(rigidBodies, i);
	    if(rb) {

	    	V3 gravity = EasyPhysics_AddGravity(rb, rb->gravityFactor);

	    ///////////////////////************* Integrate accel & velocity ************////////////////////
	    	
	        rb->dP = v3_plus(v3_scale(dt, v3_scale(rb->inverseWeight, v3_plus(v3_plus(rb->accumForce, gravity), rb->accumForceOnce))), rb->dP);

			//TODO(ollie): Integrate torque 

	        rb->dA = v3_plus(v3_scale(dt, v3_plus(rb->accumTorque, rb->accumTorqueOnce)), rb->dA);


	    ////////////////////////////////////////////////////////////////////

	        rb->dP = v3_minus(rb->dP, v3_scale(rb->dragFactor, rb->dP)); 

	        //Decay the forces so it averages out over different frame rates
			rb->accumForceOnce = NULL_VECTOR3;
    		rb->accumTorqueOnce = NULL_VECTOR3;

    		
	        ////////////////////////////////////////////////////////////////////

	        if(!(rb->isGrounded & (1 << 1))) {
	        	rb->isGrounded = 0;
	        }
	    }
	}


    for (int i = 0; i < colliders->count; ++i)
    {
        EasyCollider *a = (EasyCollider *)getElement(colliders, i);
        if(a) {

        	V3 lastPos = a->T->pos;
        	Quaternion lastQ = a->T->Q;
        ///////////////////////************* Integrate the physics ************////////////////////

	        if(a->rb && !a->rb->updated) {
	        	EasyRigidBody *rb = a->rb;
	        	a->T->pos = v3_plus(v3_scale(dt, rb->dP), a->T->pos);

	        	a->T->Q = addScaledVectorToQuaternion(a->T->Q, rb->dA, dt);

	        	a->T->Q = easyMath_normalizeQuaternion(a->T->Q);

	        	a->rb->updated = true;
	        }

        ////////////////////////////////////////////////////////////////////

            float smallestDistance = 0.1f;
            
            Array_Dynamic hitEnts;
            initArray(&hitEnts, EasyPhysics_HitBundle);

            // EasyPhysics_HitBundle bundle = {};

        ///////////////////////*********** Test for collisions **************////////////////////

            for (int j = 0; j < colliders->count; ++j)
            {
                if(i != j) {

	                EasyCollider *b = (EasyCollider *)getElement(colliders, j);

	                if(b && b->rb != a->rb && EasyPhysics_layersCanCollider(a->layer, b->layer)) {
	                	bool hit = false;
	                	/*
	                		If both objects aren't triggers, actually process the physics
	                		using minkowski-baycentric coordinates

	                	*/
	                	if(!a->isTrigger && !b->isTrigger) {
	                		/*
								Non trigger collisions
	                		*/
	                		EasyRigidBody *rb_a = a->rb;
	                		EasyRigidBody *rb_b = b->rb;

	                		if(!(rb_a->inverseWeight == 0 && rb_b->inverseWeight == 0)) 
	                		{
	                		    EasyCollisionOutput out = EasyPhysics_SolveRigidBodies(a, b);

	                		    if(out.distance <= smallestDistance && dotV2(out.normal.xy, rb_a->dP.xy) < 0.0f) {
									ArrayElementInfo arrayInfo = getEmptyElementWithInfo(&hitEnts);
									EasyPhysics_HitBundle *bundle = (EasyPhysics_HitBundle *)arrayInfo.elm;
	                		        
	                		        bundle->hitEnt = b;
	                		        bundle->outputInfo = out;
	                		    }
	                		}
	                	} else {
	                	/*
	                		If there is a trigger involved, just do overlap collision detection
	                	*/
	                		
	                		V3 aPos = v3_plus(easyTransform_getWorldPos(a->T), a->offset);
	                		V3 bPos = v3_plus(easyTransform_getWorldPos(b->T), b->offset);

	                		bool circle = a->type == EASY_COLLIDER_CIRCLE || b->type == EASY_COLLIDER_CIRCLE;

	                		bool rectangle = a->type == EASY_COLLIDER_RECTANGLE || b->type == EASY_COLLIDER_RECTANGLE;
	                		if(circle && rectangle) {
	                			assert(false);
	                		} else if(a->type == EASY_COLLIDER_RECTANGLE && b->type == EASY_COLLIDER_RECTANGLE) { //both rectangles

	                			V2 hDim = v2(0.5f*b->dim2f.x, 0.5f*b->dim2f.y);

	                			V2 points[] = { v2(-hDim.x, -hDim.y), 
	                							v2(hDim.x, -hDim.y),
	                							v2(-hDim.x, hDim.y),
	                							v2(hDim.x, hDim.y) };

	                			for(int pointI = 0; pointI < 4 && !hit; ++pointI) {
	                				V2 p = v2_plus(points[pointI], bPos.xy);
	                				if(inBounds(p, rect2fCenterDim(aPos.x, aPos.y, a->dim2f.x, a->dim2f.y), BOUNDS_RECT)) {
	                					hit = true;
	                				}
	                			}

	                			if(!hit) {
	                				hDim = v2(0.5f*a->dim2f.x, 0.5f*a->dim2f.y);

	                				V2 points2[] = { v2(-hDim.x, -hDim.y), 
	                								v2(hDim.x, -hDim.y),
	                								v2(-hDim.x, hDim.y),
	                								v2(hDim.x, hDim.y) };

	                				for(int pointI = 0; pointI < 4 && !hit; ++pointI) {
	                					V2 p = v2_plus(points2[pointI], aPos.xy);
	                					if(inBounds(p, rect2fCenterDim(bPos.x, bPos.y, b->dim2f.x, b->dim2f.y), BOUNDS_RECT)) {
	                						hit = true;
	                					}
	                				}
	                			}
	                		} else if((a->type == EASY_COLLIDER_CIRCLE && b->type == EASY_COLLIDER_CIRCLE) || (a->type == EASY_COLLIDER_SPHERE && b->type == EASY_COLLIDER_SPHERE)) { //both circles
	                			V3 centerDiff = v3_minus(aPos, bPos);

	                			//TODO(ollie): Can use radius sqr for speed!
	                			if(getLengthV3(centerDiff) <= (a->radius + b->radius)) {
	                				hit = true;
	                			}
	                		} else if ((a->type == EASY_COLLIDER_CIRCLE && b->type == EASY_COLLIDER_BOX) || (a->type == EASY_COLLIDER_BOX && b->type == EASY_COLLIDER_SPHERE)) {
	                			assert(false);
	                		} else {
	                			//case not handled
	                			assert(false);
	                		}
	                	}

	                	if(hit) {

	                		
	                		//add collision info
	                		EasyCollider_addCollisionInfo(a, b->T->id);
	                		EasyCollider_addCollisionInfo(b, a->T->id);

	                		// if((a->layer == EASY_COLLISION_LAYER_ITEMS || a->layer == EASY_COLLISION_LAYER_PLAYER) && (b->layer == EASY_COLLISION_LAYER_ITEMS || b->layer == EASY_COLLISION_LAYER_PLAYER)) {
	                		// 	char string[256];
	                		// 	sprintf(string, "%d %d", a->collisionCount, b->collisionCount);
	                		// 	easyConsole_addToStream(DEBUG_globalEasyConsole, string);
	                			
	                		// }
	                	}
	                }
	            }
            }

            V2 deltaPos = v2_minus(a->T->pos.xy, lastPos.xy);

            for (int i = 0; i < hitEnts.count; ++i)
            {
                EasyPhysics_HitBundle *bundle = (EasyPhysics_HitBundle *)getElement(&hitEnts, i);
        		if(bundle->hitEnt) {
        			//CHeck if grounded
        			V2 up = v2(0, 1);
        			
        			if(dotV2(bundle->outputInfo.normal.xy, up) > 0.5f) { //0.5f being the slope value
        				a->rb->isGrounded |= 1 << 0;
        				a->rb->isGrounded |= 1 << 1;
        			}
        			//
	        		EasyPhysics_ResolveCollisions(a->rb, bundle->hitEnt->rb, a->T, bundle->hitEnt->T, &bundle->outputInfo, lastPos, lastQ, &deltaPos);	
        		}
        	}
            a->T->pos.xy = v2_plus(lastPos.xy, deltaPos); 
        }
    }

   

}


static void EasyPhysics_UpdateWorld(EasyPhysics_World *world, float dt) {
	DEBUG_TIME_BLOCK()
	world->physicsTime += dt;
	int dividend = world->physicsTime / PHYSICS_TIME_STEP;
	float timeInterval = PHYSICS_TIME_STEP; 
	while(world->physicsTime >= timeInterval) {

		for (int i = 0; i < world->rigidBodies.count; ++i)
		{
		    EasyRigidBody *a = (EasyRigidBody *)getElement(&world->rigidBodies, i);
		    if(a) {
		    	a->updated = false;
		    }
		}
	    ProcessPhysics(&world->colliders, &world->rigidBodies, timeInterval);
	    world->physicsTime -= timeInterval;
	}

	for (int i = 0; i < world->rigidBodies.count; ++i)
	{
	    EasyRigidBody *rb = (EasyRigidBody *)getElement(&world->rigidBodies, i);
	    if(rb) {
	    	rb->accumForce = NULL_VECTOR3;
	    	rb->accumTorque = NULL_VECTOR3;	
	    }
	}
				

	for (int i = 0; i < world->colliders.count; ++i)
	{
	    EasyCollider *a = (EasyCollider *)getElement(&world->colliders, i);
	    if(a) {
	    	a->rb->isGrounded &= ~(1 << 1); //to let know it's not this frame
	   		EasyCollider_removeCollisions(a);
	    }
	}

}