#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
int main()
{
	int fd, num;

	fd = open("/dev/CDEV_TANG", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd !=    - 1)
	{
		while (1)
		{
			read(fd, &num, sizeof(int)); //bloking here,unless iput var
				printf("The globalvar is %d\n", num);

			//exit read
				if (num == 0)
				{
					close(fd);
					break;
				}
		}
	}
	else
	{
		printf("device open failure\n");
	}
	return 0;
}
