#define DEFAULT_DAY_NIGHT_SPEED 0.001f

typedef struct {
	float timeOfDay; //0 - 1,  0.5f being midday
	float timeOfDaySpeed;
} GameWeatherState;

static inline GameWeatherState *initWeatherState() {
	GameWeatherState *state = pushStruct(&globalLongTermArena, GameWeatherState);

	state->timeOfDay = 0.5f;
	state->timeOfDaySpeed = DEFAULT_DAY_NIGHT_SPEED;
	return state;
}

static inline float getTimeOfDay(GameWeatherState *w) {
	float time = 24.0f*w->timeOfDay;
	return time;
}