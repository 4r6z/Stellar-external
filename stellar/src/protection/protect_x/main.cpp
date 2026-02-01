#include "includes.h"


void mainprotect() {
	std::thread(security_loop).detach();
	std::thread(obfuscator).detach();
	std::thread(check_processes).detach();
	std::thread(vmware_check).detach();

	
}


