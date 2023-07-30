extern "C" {
  #include <unistd.h>
  #include <getopt.h>

  #include <sys/types.h>
  #include <pwd.h>
  #include <grp.h>
}

#include <iostream>

#include "AudioServer.hpp"

static struct {
    bool isDaemon = false;
    std::string user = "nobody";

    std::string ip = "0.0.0.0";
    unsigned int port = 8080;

    std::string db_path;
    bool isLive = false;

    unsigned int thread_number = 2;
} args;

void printHelp(std::ostream& out, const char* name) noexcept {
    out << "Usage: " << (name != nullptr ? name : "ng-audio-backend") << " [options]" << std::endl << std::endl
        << "Options:" << std::endl
        << "--help, -h" << std::endl
        << "\tPrint this help message." << std::endl
        << "--daemonize, -B" << std::endl
        << "\tRun this program as a service." << std::endl
        << "--user, -u <username>" << std::endl
        << "\tRun this program as another user. (default: nobody)" << std::endl
        << "--dbpath, -d <filename>" << std::endl
        << "\tSet the path of the DB file to use." << std::endl
        << "--live, -l" << std::endl
        << "\tDo not copy DB in RAM (default: false)" << std::endl
        << "--socket, -s <ip4:socket>" << std::endl
        << "\tSet the listening socket. (default: 0.0.0.0:8080)" << std::endl
        << "--thread, -t <num>" << std::endl
        << "\tSet the number of HTTP Worker. (default: 2)" << std::endl;

    std::exit(0);
}

static const char *short_option = static_cast<const char*>("hBu:d:ls:t:");
static const struct option long_options[] = {
    {"help",      no_argument,        0, 'h'},
    {"deamonize", no_argument,        0, 'B'},
    {"user",      required_argument,  0, 'u'},
    {"dbpath",    required_argument,  0, 'd'},
    {"live",      no_argument,        0, 'l'},
    {"socket",    required_argument,  0, 's'},
    {"thread",    required_argument,  0, 't'},
    {0, 0, 0, 0}
};

static void parse_args(int argc, char **argv) {
    int option_index = 0; int c;

    while (true) {
        c = getopt_long(argc, argv, short_option, long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'h':
                printHelp(std::cout, argv[0]);
                break;

            case 'B':
                args.isDaemon = true;
                break;

            case 'u':
                args.user = optarg;
                break;

            case 'd':
                args.db_path = optarg;
                break;

            case 's':
            {
                std::string tmp(optarg); std::string::size_type pos = tmp.find(':');
                if (pos == std::string::npos) {
                    std::cerr << "[-] Invalid socket format" << std::endl;
                    printHelp(std::cerr, argv[0]);
                }
                args.ip    = tmp.substr(0, pos);
                args.port  = std::stoi(tmp.substr(pos +1));
                break;
            }

            case 't':
                args.thread_number = std::atoi(optarg);
                break;

            case 'l':
                args.isLive = true;
                break;

            default:
                printHelp(std::cerr, argv[0]);
                break;
        }
    }
}

// You really have to check your privilege !
static void drop_privilege(const char* user) {
    char *buf;
    struct passwd pw;
    struct passwd *result;
    int64_t bufsize;

    if (!user || getuid() != 0)
        return;

    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
        bufsize = 16384;

    buf = new char[bufsize];
    getpwnam_r(user, &pw, buf, bufsize, &result);
    if (result == nullptr) {
        std::cerr << "[-] Can't drop root privilege..." << std::endl;
        std::exit(-1);
    }

    if (initgroups(pw.pw_name, pw.pw_gid) != 0 || setgid(pw.pw_gid) !=0 || setuid(pw.pw_uid) != 0) {
        std::cerr << "[-] Couldn't change to " << user << " uid=" << pw.pw_uid << " gid=" << pw.pw_gid <<std::endl;
        std::exit(-1);
    }

    if (setuid(0) != -1 || setgid(0) != -1) {
        std::cout << "[-] CRITICAL: Can become root again after dropping privilege..." << std::endl;
        std::exit(-1);
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    if (args.isDaemon) {
        drop_privilege(args.user.c_str());
        daemon(0, 0);
    }

    Pistache::Address addr(args.ip, args.port);
    AudioServer srv(addr);

    if (!srv.getDatabase()->openDBFile(args.db_path, args.isLive)) {
        std::cerr << "Can't open DB file. Exiting..." << std::endl;
        std::exit(-1);
    }
    srv.init(args.thread_number);
    srv.start();

    return 0;
}
