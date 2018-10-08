#include <iostream>

extern "C"
{
  #include <unistd.h> //daemon()
  #include <getopt.h>
  #include <syslog.h> //Will be use... Probably in his own object.
  
  #include <sys/types.h>
  #include <pwd.h>
  #include <grp.h>
}

#include <csignal> // signal() : To properly free all the ressources.

#include <tcp/server> //tcp::Server
#include "Audio-Bridge-Server.hpp" //Audio Worker

#include "AudioDatabase.hpp"

tcp::Server   *Server = nullptr;
AudioDatabase *db     = nullptr;

static struct {
  bool isDaemon = false;
  std::string user = "nobody";
  
  std::string ip = "0.0.0.0";
  unsigned int port = 8080;
  
  std::string db_path;
  bool isLive = false;
  
  unsigned int thread_number =
    std::thread::hardware_concurrency();
} args;

void printHelp(std::ostream& out, const char* name) noexcept
{
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
      << "--live, -s" << std::endl
      << "\tDo not copy DB in RAM (default: false)" << std::endl
      << "--socket, -s <ip4:socket>" << std::endl
      << "\tSet the listening socket. (default: 0.0.0.0:8080)" << std::endl
      << "--thread, -t <num>" << std::endl
      << "\tSet the number of HTTP Worker. (default: "
      << std::thread::hardware_concurrency() << ")" << std::endl;
      
  std::exit( 0 );
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

static void parse_args(int argc, char **argv)
{
  int option_index = 0; int c;
  
  while( true )
  {
    c = getopt_long(argc, argv, short_option, long_options, &option_index );
    
    if( c == -1 )
      break;
    
    switch( c )
    {
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
        if( pos == std::string::npos )
        {
          std::cerr << "[-] Invalid socket format" << std::endl;
          printHelp(std::cerr, argv[0]);
        }
        args.ip    = tmp.substr(0, pos);
        args.port  = std::stoi( tmp.substr(pos +1) );
        break;
      }
      
      case 't':
        args.thread_number = std::atoi( optarg );
        break;
      
      default:
        printHelp(std::cerr, argv[0]);
        break;
    }
  }
}

//You really have to check your privilege !
static void drop_privilege(const char* user)
{
  if( user && getuid() == 0 )
  {
    struct passwd *pw = getpwnam(user);
    if(pw)
    {
      if( initgroups( pw->pw_name, pw->pw_gid ) != 0 || setgid( pw->pw_gid ) !=0 || setuid( pw->pw_uid ) != 0 )
      {
        std::cerr << "[-] Couldn't change to " << user << " uid=" << pw->pw_uid << " gid=" << pw->pw_gid <<std::endl;
        std::exit(-1);
      }
      
      if( setuid(0) != -1 || setgid(0) != -1 )
      {
        std::cout << "[-] CRITICAL: Can become root again after dropping privilege..." << std::endl;
        std::exit(-1);
      }
    }
    else
    {
      std::cerr << "[-] Can't drop root privilege..." << std::endl;
      std::exit(-1);
    }
  }
}

static void signal_handler( int sig )
{
  switch( sig )
  {
    case SIGINT:  
      if( Server != nullptr )
        Server->stop();
      
      std::cout << "Server stopped..." << std::endl;
      break;
    
    case SIGUSR1:
      if(db)
        db->reload();
      std::cout << "[+] Database reloaded" << std::endl;
      break;
    
    default:;
  }
}

static void create_objects_via_args()
{
  db = new AudioDatabase();
  if( !db->openDBFile(args.db_path, args.isLive) )
  {
    std::cerr << "Can't open DB File. Exiting..." << std::endl;
    std::exit(-1);
  }
  
  Server = new tcp::Server( args.ip,
                            args.port);
  //Create Workers
  AudioServer* w = nullptr;
  
  for( unsigned int i = 0; i < args.thread_number; i++ )
  {
    w = new AudioServer();
    w->setDBController(db);
    
    Server->addWorker(w);
  }
  
  std::cout << "Server listenning on " << args.ip << ":" << args.port << std::endl;
  
  if( args.isDaemon )
  {
    drop_privilege( args.user.c_str() );
    daemon(0, 0);
  }
}

int main( int argc, char *argv[] )
{
  signal(SIGINT, signal_handler);
  
  parse_args(argc, argv);
  create_objects_via_args();
  
  std::cout << "Entering server loop..." << std::endl;
  Server->start();
  
  delete Server;
  delete db;
  
  return 0;
}
