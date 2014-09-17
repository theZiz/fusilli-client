#include <string.h>
#include <sparrowNet.h>

#define VERSION "1.1.0.0"

spNetC4AProfilePointer profile = NULL;

int check_profile()
{
	profile = spNetC4AGetProfile();
	if (profile == NULL)
	{
		printf("No profile file found!\n Please create on with a C4A Manager\n");
		return 0;
	}
	return 1;
}

int main(int argc, char **argv)
{
	spInitNet();
	printf("Fusilli client Version "VERSION"\n");
	printf("Built for "SP_DEVICE_STRING"\n");
	if (check_profile == 0)
		return -1;
	if (argc < 3)
	{
		//printf("Usage: fc GAMENAME SCORE [TIMEOUT (in ms, default is 10000)]\n");
		printf("Usage: fc [OPTIONS] ACTION [ACTION OPTIONS]\n");
		printf("These actions may be:\n");
		printf("       fc [OPTIONS] push GAMENAME SCORE [TIMEOUT]\n");
		printf("       * submits the score SCORE for the game GAMENAME using the optional\n");
		printf("         timeout TIMEOUT (in ms). The default timeout is 10000 ms.\n");
		return -1;
	}
	char* game_name = argv[1];
	int score = atoi(argv[2]);
	int time_out = 10000;
	if (argc > 3)
		time_out = atoi(argv[3]);
	printf("Try to commit the score %i to %s. Timeout in %i ms...\n",score,game_name,time_out);
	if (spNetC4ACommitScore(profile,game_name,score,NULL,time_out))
	{
		printf("Unknown error...\n");
		spNetC4AFreeProfile(profile);
		return -1;
	}
	while (spNetC4AGetStatus() == SP_C4A_PROGRESS)
	#ifdef WIN32
		Sleep(1);
	#else
		usleep(200);
	#endif
	if (spNetC4AGetStatus() != SP_C4A_OK)
	{
		printf("Transmission error...\n");
		spNetC4AFreeProfile(profile);
		return -1;
	}
	printf("Success\n");
	spNetC4AFreeProfile(profile);
	spQuitNet();
	return 0;
}
