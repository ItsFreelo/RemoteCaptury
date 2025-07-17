#include <stdio.h>
#include "RemoteCaptury.h"
#include <ctime>
// this example uses the polling interface to get the current pose of all actors from CapturyLive


int main(int argc, char** argv)
{

	// The RemoteCaptury.h file was changed in a way that all the included files need the RemoteCaptury*rc variable.
	RemoteCaptury* rc = Captury_create();

	// this is cheating a little as we're not connecting to a remote machine
	// but you can change the IP. It should work just as well.
	int ret = Captury_connect(rc, "127.0.0.1", 2101);
	if (ret == 0)
		return -1;

	// this is optional but sometimes it's quite useful to know the exact time on the CapturyLive machine.
	// The accuracy of the synchronization depends primarily on your OS and the network topology between
	// the CapturyLive machine and this machine. On a LAN running Windows on the target machine you can
	// expect about 100us accuracy.
	Captury_startTimeSynchronizationLoop(rc);

	// start streaming
	// The recommended features are CAPTURY_STREAM_POSES and CAPTURY_STREAM_COMPRESSED.
	// Additional flags should be added as required.
	Captury_startStreaming(rc, CAPTURY_STREAM_POSES | CAPTURY_STREAM_COMPRESSED);

	uint64_t lastTimestamp = 0;
	double timeSinceStart = 0.0;
	while (true) {
		// get list of actors - otherwise we won't know whom to poll
		const CapturyActor* actors;
		int numActors = Captury_getActors(rc, &actors);
		using namespace std;
		clock_t begin = clock();

		for (int i = 0; i < numActors; ++i) {

			CapturyPose* pose = Captury_getCurrentPose(rc, actors[i].id);
			if (pose == NULL)
				continue;

			if (pose->timestamp != lastTimestamp) {
				
				for(int currentJoint=0; currentJoint < pose->numTransforms; currentJoint++){
					Captury_log(rc, CAPTURY_LOG_INFO,"%u;/%s/maya", (int)timeSinceStart, actors[i].name);
					Captury_log(rc, CAPTURY_LOG_INFO,"/%s/matrix", actors[i].joints[currentJoint]);

					for(float currentTranslation : pose->transforms[currentJoint].translation){
						Captury_log(rc, CAPTURY_LOG_INFO, ";%f|f", currentTranslation);
					}

					for(float currentRotation : pose->transforms[currentJoint].rotation){
						Captury_log(rc, CAPTURY_LOG_INFO, ";%f|f", currentRotation);
					}

					Captury_log(rc, CAPTURY_LOG_INFO,"\n");
				}
				

				//Old Logging information, wich was replaced with OSC compliant data. 
				/*
				Captury_log(rc, CAPTURY_LOG_INFO, "actor %x has new pose at %zd\n", pose->actor, pose->timestamp);
				Captury_log(rc, CAPTURY_LOG_INFO, "TimeSinceStart: %u\n", (int)timeSinceStart);
				
				//This Code prints out each joint and its current position/ rotation
				Captury_log(rc, CAPTURY_LOG_INFO, "Number of Joints: %x\n", pose->numTransforms);
				for(int currentJoint=0; currentJoint < pose->numTransforms; currentJoint++){
					Captury_log(rc, CAPTURY_LOG_INFO, "joint Nr.%u, NAME:%s ", currentJoint, actors[i].joints[currentJoint]);


					for(float currentTranslation : pose->transforms[currentJoint].translation){
						Captury_log(rc, CAPTURY_LOG_INFO, "\t translation: %f", currentTranslation);
					}

					for(float currentRotation : pose->transforms[currentJoint].rotation){
						Captury_log(rc, CAPTURY_LOG_INFO, "\t rotation: %f", currentRotation);
					}

					Captury_log(rc, CAPTURY_LOG_INFO, "\n");
				}
				Captury_log(rc, CAPTURY_LOG_INFO, "End of Timestamp \n\n\n");
				*/

				clock_t end = clock();
				timeSinceStart = timeSinceStart + double(end - begin) / CLOCKS_PER_SEC * 1000;

				lastTimestamp = pose->timestamp;
			}

			// make sure to free the pose using the provided function - potential binary incompatibilities between different Microsoft compilers
			Captury_freePose(pose);
		}
	}

	// this is never called - I know.
	// Your code will obviously do this right and always clean everything up.
	Captury_stopStreaming(rc);

	Captury_disconnect(rc);

	return 0;
}
