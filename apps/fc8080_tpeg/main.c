#include <stdio.h>
#include <signal.h>

#include "monitor.h"

void exit_fun()
{
	mmi_exit_cmd(0,NULL);
}
int main()
{
	signal(SIGABRT, &exit_fun);
	signal(SIGTERM, &exit_fun);
	signal(SIGINT, &exit_fun);
	//mmi_bbm_init_cmd(0,NULL);
        Monitor();

        return 0;
}
