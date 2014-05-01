#include <string.h>
#include <sparrowNet.h>

#define VERSION "1.0.0.0"

int main(int argc, char **argv)
{
	spInitNet();
	printf("Fusilli client Version "VERSION"\n");
	printf("Built for "SP_DEVICE_STRING"\n");
	spNetC4AProfilePointer profile = spNetC4AGetProfile();
	if (profile == NULL)
	{
		printf("No profile file found!\n Please create on with a C4A Manager\n");
		return -2;
	}
	if (argc < 4)
	{
		printf("Usage: fc GAMENAME SCORE\n");
		return -1;
	}
	spNetC4AFreeProfile(profile);
	spQuitNet();
	return 0;
}
