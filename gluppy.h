#define LOGIN_URL "https://www.google.com/accounts/ServiceLogin"
#define AUTH_URL "https://www.google.com/accounts/ServiceLoginAuth"
#define MAP_URL "http://www.google.com/glm/mmap/mwmfr?hl=en"

struct galx_search {
	int pos;
	char state;
	char *target;
	char *target_end;
};
		
