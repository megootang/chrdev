#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
int main()
{
	int fd, num;

	fd = open("/dev/CDEV_TANG", O_RDWR, S_IRUSR | S_IWUSR);
	if (fd != -1)
	{
		while (1)
		{
			printf("Please input the globalvar:\n");
			scanf("%d", &num);
			write(fd, &num, sizeof(int));

			//exit write 
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
