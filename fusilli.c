#include <string.h>
#include <sparrowNet.h>

#define VERSION "1.3.0.1"

#ifdef WIN32
	#define SLEEP_MACRO Sleep(1);
#else
	#define SLEEP_MACRO usleep(200);
#endif

spNetC4AProfilePointer profile = NULL;
int test_me = 0;
int caching = 0;
int filtered = 0;
int month = 0;
int year = 0;
int ranks = 0;

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
	printf("Usage: fusilli [OPTIONS] ACTION [ACTION OPTIONS]\n");
	printf("These actions may be:\n");
	printf("       fusilli [OPTIONS] push GAMENAME SCORE [TIMEOUT]\n");
	printf("       * submits the score SCORE for the game GAMENAME using the optional\n");
	printf("         timeout TIMEOUT (in ms). The default timeout is 10000 ms.\n");
	printf("         If the timeout is 0, no score will be send, but cached if enabled.\n");
	printf("       * OPTIONS may be either --test-me, --cache, --backwardscache or\n");
	printf("         --multicach. In the first case a score is only submitted if the\n");
	printf("         score isn't uploaded at c4a yet for the player. With --multicache\n");
	printf("         the score is written to a file if the submit failed (e.g. because\n");
	printf("         of a missing network connection). It will be tried to resend it the\n");
	printf("         next time a score is submitted with fusilli. --cache and\n");
	printf("         --backwardscache do the same, but for every game only the best score\n");
	printf("         is cached. With --cache higher is better, with --backwardscache lower.\n");
	printf("       fusilli emptycache [TIMEOUT]\n");
	printf("       * submits the cached scores (if available) using the optional\n");
	printf("         timeout TIMEOUT (in ms). The default timeout is 10000 ms.\n");
	printf("       fusilli [OPTIONS] pull GAMENAME [TIMEOUT]\n");
	printf("       * gets all scores of the game GAMENAME using the optional\n");
	printf("         timeout TIMEOUT (in ms). The default timeout is 10000 ms.\n");
	printf("       * OPTIONS may be --test-me, --filtered, --MMYYYY, --thismonth or --ranks.\n");
	printf("         With --test-me a score is only shown if it is from the owner of the\n");
	printf("         profile. With --filtered for every player only the best scores is\n");
	printf("         shown. With --MMYYYY you can add a month of a year to show like\n");
	printf("         --102014 for the scores of october 2014. Use --thismonth for the\n");
	printf("         score of the recent month. If you add --rankes the ranks are shown\n");
	printf("         Every result then has 5 instead of 4 entries.\n");
	printf("       * Except for using with --test-me no profile file is needed.\n");
	printf("       * Every score persists of 4 or 5 lines: (Rank,) Player name, short player\n");
	printf("         name, score and commit unix timestamp. So the output will be a multiple\n");
	printf("         of four or five lines long, e.g.\n");
	printf("         Evil Dragon\n");
	printf("         EVD\n");
	printf("         1337\n");
	printf("         1377323529\n");
	printf("         Ziz\n");
	printf("         ZIZ\n");
	printf("         667\n");
	printf("         1410088968\n");
	printf("       fusilli info ABOUT\n");
	printf("       * prints informations about the profile of the player\n");
	printf("       * ABOUT may be longname, shortname, password, email, prid, cache or all.\n");
	printf("         For \"all\" all informations are printed linewise in the named order.\n");
	printf("Return values:\n");
	printf("       *  0 if everything went fine\n");
	printf("       * -1 (255) at submission error. This is even -1 if the submission failed,\n");
	printf("         but was cached and if no submission was done on purpose because of\n");
	printf("         --test-me.\n");
	printf("       * -2 (254) if no profile file is found, but needed\n");
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
	int result;
	if (month)
		result = spNetC4AGetScoreOfMonth(&scoreList,test_me?profile:NULL,game_name,year,month,time_out);
	else
		result = spNetC4AGetScore(&scoreList,test_me?profile:NULL,game_name,time_out);
	if (result)
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
	if (filtered)
		spNetC4AFilterScore(&scoreList);
	spNetC4AScorePointer mom = scoreList;
	while (mom)
	{
		if (ranks)
			printf("%i\n%s\n%s\n%i\n%i\n",mom->rank,mom->longname,mom->shortname,mom->score,(Uint32)(mom->commitTime));
		else
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
	if (need_to_quit(mom_field,argc))
		return -1;
	while (argv[mom_field][0] == '-' && argv[mom_field][1] == '-') //Starts with --
	{
		if (strcmp(argv[mom_field],"--test-me") == 0)
			test_me = 1;
		else
		if (strcmp(argv[mom_field],"--multicache") == 0)
			caching = 1;
		else
		if (strcmp(argv[mom_field],"--cache") == 0)
			caching = 2;
		else
		if (strcmp(argv[mom_field],"--backwardscache") == 0)
			caching = 3;
		else
		if (strcmp(argv[mom_field],"--filtered") == 0)
			filtered = 1;
		else
		if (strcmp(argv[mom_field],"--ranks") == 0)
			ranks = 1;
		else
		if (strcmp(argv[mom_field],"--thismonth") == 0)
		{
			time_t rawtime;
			struct tm * ptm;
			time ( &rawtime );
			ptm = gmtime ( &rawtime );	
			year = ptm->tm_year+1900;
			month = ptm->tm_mon+1;
		}
		else
		{
			//Assuming MMYYYY
			char month_c[3];
			memcpy(month_c,&(argv[mom_field][2]),2);
			month_c[2] = 0;
			month = atoi(month_c);
			char year_c[5];
			memcpy(year_c,&(argv[mom_field][4]),4);
			year_c[4] = 0;
			year = atoi(year_c);
			if (month < 1)
				month = 1;
			if (month > 12)
				month = 12;
		}
		mom_field++;
		if (need_to_quit(mom_field,argc))
			return -1;
	}
	if (test_me && caching)
	{
		printf("Error: --test-me and --cache doesn't work together\n");
		spQuitNet();
		return -1;
	}
	if (need_to_quit(mom_field,argc))
		return -1;
	spNetC4ASetCaching(caching);
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
