#include <stacsos/kernel/debug.h>
#include <stacsos/kernel/log.h>

using namespace stacsos::kernel;

logger logger::root_logger;

static const char *log_level_text[] = { "trace", "debug", "info", "warning", "error", "panic" };

void logger::emit(const char *log_source, log_level level, const char *message)
{
	dprintf("[%s] %s: %s\n", log_level_text[(unsigned int)level], log_source, message);
}
