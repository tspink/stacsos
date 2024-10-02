#include <stacsos/kernel/config.h>
#include <stacsos/kernel/debug.h>

using namespace stacsos::kernel;

void config::init(const char *cmdline)
{
	memops::strncpy(command_line_, cmdline, 256);

	char *p = command_line_;
	if (!*p) {
		return;
	}

	int state = 0;
	const char *kp = p;
	const char *vp = "";
	while (*p) {
		switch (state) {
		case 0:
			if (*p == '=') {
				*p = 0;
				vp = p + 1;
				state = 1;
			} else if (*p == ' ') {
				*p = 0;
				add_option(kp, "");
				kp = p + 1;
			}
			break;

		case 1:
			if (*p == ' ') {
				*p = 0;

				add_option(kp, vp);

				kp = p + 1;
				state = 0;
			}
			break;
		}

		p++;
	}

	add_option(kp, vp);
}

void config::add_option(const char *key, const char *value)
{
	options_[nr_options_].key = key;
	options_[nr_options_].value = value;
	nr_options_++;
}
