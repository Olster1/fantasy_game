typedef struct {
	float timeOfDay; //0 - 1,  0.5f being midday
	float timeOfDaySpeed;
} GameWeatherState;

static inline GameWeatherState *initWeatherState() {
	GameWeatherState *state = pushStruct(&globalLongTermArena, GameWeatherState);

	state->timeOfDay = 1.f;
	state->timeOfDaySpeed= 0.1f;
	return state;
}