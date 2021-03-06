#include "pt.h"
#include <iostream>

uint64_t min_addr_cle, max_addr_cle, entry_point_cle;
uint8_t* raw_bin_buf;
uint8_t * trace_bits;                /* SHM with instrumentation bitmap  */

bool read_min_max()
{
    FILE *fp;
    int MAX_LINE_T = 1024;
    char strLine[MAX_LINE_T];
    char* endptr;

    min_addr_cle = 0ULL;
    max_addr_cle = 0ULL;
    entry_point_cle = 0ULL;

    if((fp = fopen("../min_max.txt","r")) == NULL)
    {
        return false;
    }
    fgets(strLine,MAX_LINE_T,fp);
    min_addr_cle = strtoull(strLine, &endptr, 10);
    fgets(strLine,MAX_LINE_T,fp);
    max_addr_cle = strtoull(strLine, &endptr, 10);
    fgets(strLine,MAX_LINE_T,fp);
    entry_point_cle = strtoull(strLine, &endptr, 10);
    fclose(fp);

    if(min_addr_cle == 0ULL || max_addr_cle == 0ULL || entry_point_cle == 0ULL)
    {
        printf("Error: min max addr = 0\n");
        return false;
    }
    return true;
}

bool read_raw_bin()
{
    FILE* pt_file = fopen("../raw_bin", "rb");

    raw_bin_buf = (uint8_t*)malloc(max_addr_cle - min_addr_cle);
    memset(raw_bin_buf, 0, max_addr_cle - min_addr_cle);

    if(NULL == pt_file)
    {
        return false;
    }

    int count;
    while (!feof (pt_file))
    {
        count = fread (raw_bin_buf, sizeof(uint8_t), max_addr_cle - min_addr_cle, pt_file);
    }
    fclose(pt_file);
    return true;
}

int main(int argc, char** argv)
{
    if(argc < 5) {
        std::cout << argv[0] << " <raw_bin> <min_addr> <max_addr> <entry_point>" << std::endl;
        exit(0);
    }
    char* raw_bin = argv[1];
    uint64_t min_addr = strtoul(argv[2], nullptr, 0);
    uint64_t max_addr = strtoul(argv[3], nullptr, 0);
    uint64_t entry_point = strtoul(argv[4], nullptr, 0);

	pt_fuzzer fuzzer(raw_bin, min_addr, max_addr, entry_point);
	fuzzer.init();


    pid_t pid;        //进程标识符
	pid = fork();     //创建一个新的进程
	if(pid < 0)
	{
		printf("创建进程失败!");
		exit(1);
	}
	else if(pid == 0)   //如果pid为0则表示当前执行的是子进程
	{
		std::cout << "child process start, pid is " << getpid() << "." << std::endl;
		sleep(1);
        //std::string bin_file = "/home/guy/ptfuzzer/afl-pt/ptest/readelf";
        //std::string args = "-a /home/guy/ptfuzzer/afl-pt/ptest/in/small_exec.elf";
        char* args[4] = {"test", "-a", "/home/zhouxu/ptfuzzer/afl-pt/ptest/in/small_exec.elf", nullptr};
		int ret = execv("./test", args);
	    if(ret == -1){
            std::cerr << "execv failed." << std::endl;
            exit(-1);
        }
    }

	else          //否则为父进程
	{
		printf("这是父进程,进程标识符是%d\n",getpid());
		fuzzer.start_pt_trace(pid);
		int status;
		waitpid(pid, &status, 0);
        uint8_t *a;
        a = (uint8_t*)malloc(MAP_SIZE * sizeof(uint8_t));
		fuzzer.stop_pt_trace(a);
        printf("\n\n");
	}

	return 0;
}
