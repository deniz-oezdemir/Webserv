#pragma once

namespace signals
{
/* The server handles the following signals:
SIGINT, SIGQUIT, SIGTERM, SIGPIPE, SIGHUP */
void handleSignals(void);
} // namespace signals
