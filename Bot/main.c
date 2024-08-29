/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char** argv){
	const char* fn = argv[1] == NULL ? "archiver.ini" : argv[1];
	FILE* f = fopen(fn, "r");
	if(f == NULL){
		fprintf(stderr, "Could not open the config: %s\n", fn);
		return 1;
	}

	struct stat s;
	stat(fn, &s);

	char* buf = malloc(s.st_size + 1);
	fread(buf, s.st_size, 1, f);
	buf[s.st_size] = 0;

	int i;
	int incr = 0;

	for(i = 0;; i++){
		if(buf[i] == 0 || buf[i] == '\n'){
			char oldc = buf[i];
			buf[i] = 0;
			char* line = buf + incr;
			if(strlen(line) > 0){
				int j;
				for(j = 0; line[j] != 0; j++){
					if(line[j] == '='){
						line[j] = 0;
						char* key = line;
						char* value = line + j + 1;



						break;
					}
				}
			}
			incr = i + 1;
			if(oldc == 0) break;
		}else{
		}
	}

	free(buf);
	fclose(f);
}
