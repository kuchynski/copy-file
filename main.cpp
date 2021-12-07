
#include "file.h"

int main(int argc, char** argv)
{
	// argv[1] - the source file
	// argv[2] - the destination file
	int ret = -1;
	
	if(argc > 2) {
		File file(argv[1]);
	
		if(file) {
			if(file.copy_to(argv[2])) {
				ret = 0;
			}
		}
		
	}
	
	return ret;
}
