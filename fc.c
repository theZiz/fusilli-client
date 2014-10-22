#include <string.h>
#include <sparrowNet.h>

#define VERSION "1.1.1.1"

#ifdef WIN32
	#define SLEEP_MACRO Sleep(1);
#else
	#define SLEEP_MACRO usleep(200);
#endif

spNetC4AProfilePointer profile = NULL;
int test_me = 0;

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

void print_help()
{
	printf("Fusilli client Version "VERSION"\n");
	printf("Built for "SP_DEVICE_STRING"\n");
	printf("Usage: fc [OPTIONS] ACTION [ACTION OPTIONS]\n");
	printf("These actions may be:\n");
	printf("       fc [OPTIONS] push GAMENAME SCORE [TIMEOUT]\n");
	printf("       * submits the score SCORE for the game GAMENAME using the optional\n");
	printf("         timeout TIMEOUT (in ms). The default timeout is 10000 ms.\n");
	printf("       * OPTIONS may be --test-me or --cache. In the first case a score is only\n");
	printf("         submitted if the score isn't uploaded at c4a yet for the player.\n");
	printf("         With --cache the score is written to a file if the submit failed\n");
	printf("         (e.g. because of a missing network connection). It will be tried\n");
	printf("         to be resend the next time a score is submitted with fc.\n");
	printf("       fc emptycache [TIMEOUT]\n");
	printf("       * submits the cached scores (if available) using the optional\n");
	printf("         timeout TIMEOUT (in ms). The default timeout is 10000 ms.\n");
	printf("       fc [OPTIONS] pull GAMENAME [TIMEOUT]\n");
	printf("       * gets all scores of the game GAMENAME using the optional\n");
	printf("         timeout TIMEOUT (in ms). The default timeout is 10000 ms.\n");
	printf("       * OPTIONS may only be --test-me atm. In that case a score is only\n");
	printf("         shown if it is from the owner of the profile.\n");
	printf("       * Except for using with --test-me no profile file is needed.\n");
	printf("       * Every score persists of four lines: Player name, short player name,\n");
	printf("         score and commit unix timestamp. So the output will be a multiple of\n");
	printf("         four lines long, e.g.\n");
	printf("         Evil Dragon\n");
	printf("         EVD\n");
	printf("         1337\n");
	printf("         1377323529\n");
	printf("         Ziz\n");
	printf("         ZIZ\n");
	printf("         667\n");
	printf("         1410088968\n");
	printf("       fc info ABOUT\n");
	printf("       * prints informations about the profile of the player\n");
	printf("       * ABOUT may be longname, shortname, password, email, prid, cache or all.\n");
	printf("         For \"all\" all informations are printed linewise in the named order.\n");
}

int need_to_quit(int mom_field,int argc)
{
		if (mom_field >= argc)
		{
			printf("Error: Too few arguments!\n");
			if (profile)
				spNetC4AFreeProfile(profile);
			spQuitNet();
			return 1;
		}
		return 0;
}

int push(int mom_field,int argc,char **argv)
{
	if (check_profile() == 0)
	{
		spQuitNet();
		return -2;
	}
	char* game_name = argv[mom_field++];
	if (need_to_quit(mom_field,argc))
	{
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	int score = atoi(argv[mom_field++]);
	int time_out = 10000;
	if (argc > mom_field)
		time_out = atoi(argv[mom_field]);
	printf("Try to commit the score %i to %s. Timeout in %i ms...\n",score,game_name,time_out);
	spNetC4AScorePointer scoreList = NULL;
	if (test_me)
	{
		if (spNetC4AGetScore(&scoreList,profile,game_name,time_out))
		{
			printf("Unknown error...\n");
			spNetC4AFreeProfile(profile);
			spQuitNet();
			return -1;
		}
		while (spNetC4AGetStatus() == SP_C4A_PROGRESS)
			SLEEP_MACRO
		if (spNetC4AGetStatus() != SP_C4A_OK)
		{
			printf("Transmission error...\n");
			spNetC4AFreeProfile(profile);
			spQuitNet();
			return -1;
		}
	}
	if (spNetC4ACommitScore(profile,game_name,score,&scoreList,time_out))
	{
		printf("Unknown error...\n");
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	while (spNetC4AGetStatus() == SP_C4A_PROGRESS)
		SLEEP_MACRO
	if (spNetC4AGetStatus() != SP_C4A_OK)
	{
		printf("Transmission error...\n");
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	printf("Success\n");
	spNetC4AFreeProfile(profile);
	spQuitNet();
	return 0;
}

int emptycache(int mom_field,int argc,char **argv)
{
	if (check_profile() == 0)
	{
		spQuitNet();
		return -2;
	}
	int time_out = 10000;
	if (argc > mom_field)
		time_out = atoi(argv[mom_field]);
	printf("Try to commit the cached scores. Timeout in %i ms...\n",time_out);
	if (spNetC4ACommitScore(profile,"",0,NULL,time_out))
	{
		printf("Unknown error...\n");
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	while (spNetC4AGetStatus() == SP_C4A_PROGRESS)
		SLEEP_MACRO
	if (spNetC4AGetStatus() != SP_C4A_OK)
	{
		printf("Transmission error...\n");
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	printf("Success\n");
	spNetC4AFreeProfile(profile);
	spQuitNet();
	return 0;
}

int pull(int mom_field,int argc,char **argv)
{
	if (test_me && check_profile() == 0)
	{
		spQuitNet();
		return -2;
	}
	char* game_name = argv[mom_field++];
	int time_out = 10000;
	if (argc > mom_field)
		time_out = atoi(argv[mom_field]);
	spNetC4AScorePointer scoreList = NULL;
	if (spNetC4AGetScore(&scoreList,test_me?profile:NULL,game_name,time_out))
	{
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	while (spNetC4AGetStatus() == SP_C4A_PROGRESS)
		SLEEP_MACRO
	if (spNetC4AGetStatus() != SP_C4A_OK)
	{
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	spNetC4AScorePointer mom = scoreList;
	while (mom)
	{
		printf("%s\n%s\n%i\n%i\n",mom->longname,mom->shortname,mom->score,(Uint32)(mom->commitTime));
		mom = mom->next;
	}
	spNetC4ADeleteScores(&scoreList);	
	if (profile)
		spNetC4AFreeProfile(profile);
	spQuitNet();
	return 0;
}

int info(int mom_field,int argc,char **argv)
{
	if (check_profile() == 0)
	{
		spQuitNet();
		return -2;
	}
	if (strcmp(argv[mom_field],"all") == 0)
	{
		printf("%s\n",profile->longname);
		printf("%s\n",profile->shortname);
		printf("%s\n",profile->password);
		printf("%s\n",profile->email);
		printf("%s\n",profile->prid);
		printf("%i\n",spNetC4AHowManyCached());
	}
	else
	if (strcmp(argv[mom_field],"longname") == 0)
		printf("%s\n",profile->longname);
	else
	if (strcmp(argv[mom_field],"shortname") == 0)
		printf("%s\n",profile->shortname);
	else
	if (strcmp(argv[mom_field],"password") == 0)
		printf("%s\n",profile->password);
	else
	if (strcmp(argv[mom_field],"email") == 0)
		printf("%s\n",profile->email);
	else
	if (strcmp(argv[mom_field],"prid") == 0)
		printf("%s\n",profile->prid);
	else
	if (strcmp(argv[mom_field],"cache") == 0)
		printf("%i\n",spNetC4AHowManyCached());
	else
	{
		spNetC4AFreeProfile(profile);
		spQuitNet();
		return -1;
	}
	spNetC4AFreeProfile(profile);
	spQuitNet();
	return 0;
}

int main(int argc, char **argv)
{
	spInitNet();
	if (argc < 2)
	{
		print_help();
		spQuitNet();
		return -1;
	}
	int mom_field = 1;
	//easiest method to check for any combination of
	//--test-me
	//--test-me --cache
	//--cache
	//--cache --test-me
	if (strcmp(argv[mom_field],"--test-me") == 0)
		{ test_me = 1; mom_field++; }
	if (strcmp(argv[mom_field],"--cache") == 0)
		{ spNetC4ASetCaching(1); mom_field++; }
	if (test_me == 0 && strcmp(argv[mom_field],"--test-me") == 0)
		{ test_me = 1; mom_field++; }
	char* action = argv[mom_field++];
	if (strcmp(action,"emptycache") == 0)
		return emptycache(mom_field,argc,argv);
	else
	if (need_to_quit(mom_field,argc))
		return -1;
	if (strcmp(action,"push") == 0)
		return push(mom_field,argc,argv);
	if (strcmp(action,"pull") == 0)
		return pull(mom_field,argc,argv);
	if (strcmp(action,"info") == 0)
		return info(mom_field,argc,argv);
	spQuitNet();	
	return -1;
}
