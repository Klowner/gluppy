/*
 gcc `pkg-config --cflags --libs glib-2.0 liblocation hildon-1 libcurl dbus-glib-1` gluppy.c -o gluppy
*/
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "latitude.h"

CURL *curl = NULL;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
	printf(buffer);
	return size;
}

size_t handle_galx(void *buffer, size_t size, size_t nmemb, void *data) {
	struct galx_search *galx = (struct galx_search*)data;
	char pattern[] = "name=\"GALX\"value=\"\0";
	unsigned int data_length = size * nmemb;
	unsigned char *ptr = (unsigned char*)buffer;
	
	int i;
	for(i = 0; i < data_length; i++) {
		// COPY MODE
		if(galx->state == 'c') {
			if(*ptr == '"') {
				galx->state = 'd'; // SWITCH TO DONE MODE
				*galx->target_end = '\0';
			} else {
				*galx->target_end = *ptr;
				galx->target_end++;
			}
		}
		
		// IGNORE WHITESPACE CHARACTERS
		// SEARCH MODE
		if((int)*ptr > 32 && galx->state == 's') {	
			if( pattern[galx->pos] == *ptr) {
				galx->pos += 1;		
				if(galx->pos == strlen(pattern)) {
					galx->state = 'c'; // COPY MODE
				}
			} else {
				galx->pos = 0; // not whitespace, so reset the match count		
			}
		}
		
		++ptr;
	}
	
	if(galx->state == 'd')
		return 0;
	
	return size * nmemb;
}

char* get_galx_code(CURL *easyhandle) {
	struct galx_search galx_s;
	galx_s.target = (char*)malloc(sizeof(char)*20);
	galx_s.target_end = galx_s.target;
	galx_s.pos = 0;
	galx_s.state = 's';
	memset(&galx_s.target[0], '\0', sizeof(char)*20);
	
	curl_easy_setopt(easyhandle, CURLOPT_URL, LOGIN_URL);
	curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, handle_galx);
	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &galx_s);
	curl_easy_setopt(easyhandle, CURLOPT_SSL_VERIFYPEER, 0);

	curl_easy_perform(easyhandle);
	return galx_s.target;
}

int latitude_authenticate(CURL *curl, char *username, char *password) {
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;

	char *galx_code = get_galx_code(curl);

	curl_formadd(&formpost, &lastptr,
					CURLFORM_COPYNAME,		"GALX",
					CURLFORM_COPYCONTENTS,	galx_code,
					CURLFORM_END);
	
	curl_formadd(&formpost, &lastptr,
					CURLFORM_COPYNAME,		"Email",
					CURLFORM_COPYCONTENTS,	username,
					CURLFORM_END);
	
	curl_formadd(&formpost, &lastptr,
					CURLFORM_COPYNAME,		"Passwd",
					CURLFORM_COPYCONTENTS,	password,
					CURLFORM_END);

	//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_URL, AUTH_URL);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
	
	free(galx_code);
	
	return curl_easy_perform(curl);
}

void latitude_set_location(CURL *curl, int *acc, float *lat, float *lng) {
	time_t now;	
	struct curl_slist *chunk = NULL;
	int resp;
	char acc_str[16];
	char lat_str[16];
	char lng_str[16];
	char time_str[32];
	char post_buffer[256];
	char *ptr = &post_buffer[0];
	
	sprintf(acc_str, "%d", *acc);
	sprintf(lat_str, "%2.8f", *lat);
	sprintf(lng_str, "%2.8f", *lng);
    
	now = time(NULL);
	sprintf(time_str, "%lu000", now);
	
	chunk = curl_slist_append(chunk, "User-agent: Gluppy 0.1");
	chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded;charset=UTF-8");
	chunk = curl_slist_append(chunk, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	chunk = curl_slist_append(chunk, "Referer: http://www.google.com/maps/m?mode=latitude");
	chunk = curl_slist_append(chunk, "Accept-Encoding: gzip,deflate");
	chunk = curl_slist_append(chunk, "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7");
	chunk = curl_slist_append(chunk, "X-ManualHeader: true");

	strcpy(ptr, "accuracy=");		ptr += strlen("accuracy=");
	strcpy(ptr, acc_str);			ptr += strlen(acc_str);
	
	strcpy(ptr, "&lat=");			ptr += strlen("&lat=");
	strcpy(ptr, lat_str);			ptr += strlen(lat_str);

	strcpy(ptr, "&lng=");			ptr += strlen("&lng=");
	strcpy(ptr, lng_str);			ptr += strlen(lng_str);
	
	strcpy(ptr, "&cts=");			ptr += strlen("&cts=");
	strcpy(ptr, time_str);			ptr += strlen(time_str);
	
	strcpy(ptr, "&t=ul&auto=true");		
	ptr += strlen("&t=ul&auto=true");
	
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &post_buffer[0]);		
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	curl_easy_setopt(curl, CURLOPT_URL, MAP_URL);	
	//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	resp = curl_easy_perform(curl);
}	


