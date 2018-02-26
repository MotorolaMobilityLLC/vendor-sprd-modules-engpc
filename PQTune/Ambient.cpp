#include <utils/Log.h>
#include <utils/String16.h>
#include <utils/Trace.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <inttypes.h>
#include <android/sensor.h>
#include <utils/Looper.h>
#include "Ambient.h"


int getAmbient(uint32_t *value)
{
	uint32_t sensorRate = 20 * 1000; // us
	ASensorEvent buffer[1];
	static ASensorManager *mSensorManager;
	ssize_t n;

	ENG_LOG("PQ Ambient enter\n");
	mSensorManager = ASensorManager_getInstanceForPackage("");
	if (mSensorManager == NULL) {
		ENG_LOG("can not get ISensorManager service");
	}
	const ASensor *lightSensor = ASensorManager_getDefaultSensor(mSensorManager, ASENSOR_TYPE_LIGHT);
	ALooper *mLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	ASensorEventQueue *sensorEventQueue = ASensorManager_createEventQueue(mSensorManager, mLooper, 0, NULL, NULL);
	if (lightSensor!= NULL) {
		if (ASensorEventQueue_registerSensor(sensorEventQueue, lightSensor, sensorRate, 0) < 0) {
			ENG_LOG("Unable to register sensor ");
		}
	}

	ASensorEventQueue_enableSensor(sensorEventQueue,  lightSensor);
	while(1) {
		if((n = ASensorEventQueue_getEvents(sensorEventQueue, buffer, 1)) > 0) {
			*value = uint32_t(buffer[0].light);
			ENG_LOG("PQ Ambient is f_value = %f d_value = %d \n", buffer[0].light, *value);
			break;
		} else
			ENG_LOG("PQ Ambient gets fail\n");
	}
	ASensorEventQueue_disableSensor(sensorEventQueue,  lightSensor);
	ASensorManager_destroyEventQueue(mSensorManager, sensorEventQueue);
	ENG_LOG("PQ Ambient exit %d\n",*value);
	return 0;
}

