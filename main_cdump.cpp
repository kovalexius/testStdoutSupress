#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

class StandartIOStreamRedirector
{
public:
        StandartIOStreamRedirector()
        {
                // это C++
                
		// сохранить предыдущие значения
                m_cout_buff = std::cout.rdbuf();
                m_cerr_buff = std::cerr.rdbuf();

                // перенаправить C++ на глобальный streambuf
                //std::cout.rdbuf(m_local.rdbuf());
                //std::cerr.rdbuf(m_local.rdbuf());
                std::cout.rdbuf(nullptr);
                std::cerr.rdbuf(nullptr);

                // А это C

		// Сохранить
                m_stdout_fd = dup(fileno(stdout));
                m_stderr_fd = dup(fileno(stderr));
                
		// перенаправить C на /dev/null
                //stdout = fopen("/dev/null", "w");
                //stderr = fopen("/dev/null", "w");
		m_reopen_stdout = freopen("/dev/null", "w", stdout);
                m_reopen_stderr = freopen("/dev/null", "w", stderr);

                if(m_reopen_stdout == NULL || m_reopen_stderr == NULL)
                {
                        printf("can't open /dev/null");
                }
        }
        ~StandartIOStreamRedirector()
        {
                // Восстановить C++
                std::cout.rdbuf(m_cout_buff);
                std::cerr.rdbuf(m_cerr_buff);

		std::cout << "m_local_cout content: " << m_local_cout.str() << std::endl;
		std::cout << "m_local_cerr content: " << m_local_cerr.str() << std::endl;

                // Восстановить C, для этого:
                // закрыть дескрипторы на /dev/null
                //fclose(stdout);
                //fclose(stderr);
                // и восстановить стандартные дескрипторы:
                //stdout = m_stdout;
                //stderr = m_stderr;
        }
private:
        std::streambuf* m_cout_buff;
        std::streambuf* m_cerr_buff;
        std::ostringstream m_local_cout;
	std::ostringstream m_local_cerr;
        
	FILE*           m_reopen_stdout;
        FILE*           m_reopen_stderr;
	FILE*           m_stdout;
        FILE*           m_stderr;
	int	m_stdout_fd;
	int	m_stderr_fd;
};


void testFromCppReference()
{
	std::ostringstream local_cout;
	std::ostringstream local_cerr;

	// Save
	auto cout_buff = std::cout.rdbuf();
	auto cerr_buff = std::cerr.rdbuf();

	// substitute
	std::cout.rdbuf(local_cout.rdbuf());
	std::cerr.rdbuf(local_cerr.rdbuf());

	// try to print
	std::cout << "privet cout" << std::endl;
	std::cerr << "privet cerr" << std::endl;
	
	printf("Preved printf\n");
	fprintf(stderr, "prevet stderr\n");


	// go back
	std::cout.rdbuf(cout_buff);
	std::cerr.rdbuf(cerr_buff);


	std::cout << "local_cout content: " << local_cout.str() << std::endl;
	std::cout << "local_cerr content: " << local_cerr.str() << std::endl;
}

void testPlainReopen()
{
	// Save
	int fd_stdout = fileno(stdout);
	int fd_stderr = fileno(stderr);

	// Replace
	//*
	FILE* fp_stdout;
	FILE* fp_stderr;
	if((fp_stdout = freopen("log.txt", "w", stdout)) == NULL)
	{
		printf("Cannot reopen stdout");
	}

	if((fp_stderr = freopen("log.txt", "w", stderr)) == NULL)
	{
		printf("Cannot reopen stderr");
	}
	/**/

	// try to output
	{
	fprintf(stdout, "this stdout must be written to dev null\n");
	fprintf(stderr, "this stderr must be written to dev null\n");	
	char msg_1[] = "this 1 must be written to dev null\n";
	char msg_2[] = "this 2 must be written to dev null\n";
	write(1, msg_1, sizeof(msg_1));
	write(2, msg_2, sizeof(msg_2));
	}

	// rollback
	//*
	if(freopen("/proc/self/fd/1", "w", stdout) == NULL)
	{
		printf("cannot open /proc/self/fd/1");
	}
	if(freopen("/proc/self/fd/2", "w", stderr) == NULL)
	{
		printf("Cannot open /proc/self/fd/2");
	}
	/**/
	if((stdout = fdopen(fd_stdout, "w")) == NULL)
	{
		printf("Cannot fdopen fd_stdout\n");
	}

	if((stderr = fdopen(fd_stderr, "w")) == NULL)
	{
		printf("Cannot fdopen fd_sdterr\n");
	}

	// try to output after rollback
	{
	fprintf(stdout, "this stdout must be written to stdout\n");
	fprintf(stderr, "this stderr must be written to stderr\n");
	char msg_1[] = "this 1 must be written to stdout\n";
	char msg_2[] = "this 2 must be written to stderr\n";
	write(fileno(stdout), msg_1, sizeof(msg_1));
	write(fileno(stderr), msg_2, sizeof(msg_2));
	}
}

void testPlainReplaceStreams()
{
	// Save
	FILE* original_stdout;
	FILE* original_stderr;
	original_stdout = stdout;
	original_stderr = stderr;

	// Replace
	stdout = stderr = fopen("null.txt", "w");

	// try to output
	{
	fprintf(stdout, "this stdout must be written to dev null\n");
	fprintf(stderr, "this stderr must be written to dev null\n");	
	char msg_1[] = "this 1 must be written to dev null\n";
	char msg_2[] = "this 2 must be written to dev null\n";
	write(1, msg_1, sizeof(msg_1));
	write(2, msg_2, sizeof(msg_2));
	}

	// rollback
	fclose(stdout);
	stdout = original_stdout;
	stderr = original_stderr;


	// try to output after rollback
	{
	fprintf(stdout, "this stdout must be written to stdout\n");
	fprintf(stderr, "this stderr must be written to stderr\n");
	char msg_1[] = "this 1 must be written to stdout\n";
	char msg_2[] = "this 2 must be written to stderr\n";
	write(fileno(stdout), msg_1, sizeof(msg_1));
	write(fileno(stderr), msg_2, sizeof(msg_2));
	}
}

void testRaiiImplementation()
{
	StandartIOStreamRedirector raiiObj;

	// try to print
	std::cout << "privet cout" << std::endl;
	std::cerr << "privet cerr" << std::endl;
	
	printf("Preved printf\n");
	fprintf(stderr, "prevet stderr\n");
}



int main()
{
	//testFromCppReference();
	//testPlainReopen();
	//testPlainReplaceStreams();
	testRaiiImplementation();
	
	return 0;
}
