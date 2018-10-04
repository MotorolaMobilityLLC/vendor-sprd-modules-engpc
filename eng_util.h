#ifndef _ENG_UTIL_H_
#define _ENG_UTIL_H_

int eng_open_dev(char* dev, int mode);
int write_to_host_diag(char* nmea, int length);
int parse_text_delims(char* input_text,char *output_text1,char *output_text2,char *delims);

#endif /*!_ENG_UTIL_H_*/
