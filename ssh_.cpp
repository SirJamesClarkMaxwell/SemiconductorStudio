#include <libssh/libssh.h>
#include <iostream>

int authenticate_user(ssh_session session) {


	// later i will add logic, now focus on the concept
	return SSH_AUTH_SUCCESS;
}

int main() {

	// ssh_threads_set_callbacks(ssh_threads_get_pthread());

	if (ssh_init() != SSH_OK) {
		std::cerr << "ERROR DURING THE INITIALIZATION";

		return 1;
	}
	ssh_session session = ssh_new();
	if (session == nullptr) {
		std::cerr << "ERROR DURING CREATION OF SESSION";
		return 1;
	}
	if (session) < 0) {
		std::cerr << "Error starting SSH server" << std::endl;
		return 1;
	}

	ssh_options_set(session, SSH_OPTIONS_BINDADDR, "0.0.0.0");
	
	while (true) {
		ssh_session client_session = ssh_new();
		
	}


}