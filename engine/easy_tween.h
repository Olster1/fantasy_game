inline static float easyTween_easeInSine(float start, float end, float x) {
	float t = 1 - cos((x * PI) / 2);
	float result = start + (end - start)*t;
	return result;
}

inline static float easyTween_easeOutSine(float start, float end, float x) {
	float t = sin((x * PI) / 2); 
	float result = start + (end - start)*t;
	return result;
}

inline static float easyTween_easeInOutSine(float start, float end, float x) {
	float t = -(cos(PI * x) - 1) / 2;
	float result = start + (end - start)*t;
	return result;
}


inline static float easyTween_easeInCubic(float start, float end, float x) {
	float t = x * x * x;
	float result = start + (end - start)*t;
	return result;
}


inline static float easyTween_easeOutCubic(float start, float end, float x) {
	float t = 1 - pow(1 - x, 3);
	float result = start + (end - start)*t;
	return result;
}

inline static float easyTween_easeInOutCubic(float start, float end, float x) {
	float t = (x < 0.5) ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
	float result = start + (end - start)*t;
	return result;
}

inline static float easyTween_easeInQuint(float start, float end, float x) {
	float t = x * x * x * x * x;
	float result = start + (end - start)*t;
	return result;
}

inline static float easyTween_easeOutQuint(float start, float end, float x) {
	float t = 1 - pow(1 - x, 5);
	float result = start + (end - start)*t;
	return result;
}

inline static float easyTween_easeInOutQuint(float start, float end, float x) {
	float t = x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
	float result = start + (end - start)*t;
	return result;
}
inline static float easyTween_easeInCirc(float start, float end, float x) {
	float t = 1 - sqrt(1 - pow(x, 2));
	float result = start + (end - start)*t;
	return result;
}
inline static float easyTween_easeOutCirc(float start, float end, float x) {
	float t = sqrt(1 - pow(x - 1, 2));
	float result = start + (end - start)*t;
	return result;
}
inline static float easyTween_easeInOutCirc(float start, float end, float x) {
	float t = (x < 0.5) ? (1 - sqrt(1 - pow(2 * x, 2))) / 2 : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
	float result = start + (end - start)*t;
	return result;
}
inline static float easyTween_easeInElastic(float start, float end, float x) {
	float c4 = (2.0f * Math.PI) / 3.0f;
	float t = 0;
	if(x == 0) {

	} else if(x == 1) {
		t = 1;
	} else {
		t = -pow(2, 10 * x - 10) * sin((x * 10 - 10.75) * c4);	
	}
	float result = start + (end - start)*t;
	return result;
}


