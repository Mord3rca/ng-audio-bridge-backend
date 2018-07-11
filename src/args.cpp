#include "args.hpp"

static const struct option long_options[] = {
  {"help",      no_argument,        0, 'h'},
  {"deamonize", no_argument,        0, 'B'},
  {"user",      required_argument,  0, 'u'},
  {"group",     required_argument,  0, 'g'},
  {"dbpath",    required_argument,  0, 'd'},
  {"socket",    required_argument,  0, 's'},
  {0, 0, 0, 0}
};

static const char *short_option = static_cast<const char*>("hBu:g:d:s:");

void printHelp(std::ostream& out, const char* name) noexcept
{
  out << "Usage: " << (name != nullptr ? name : "ng-audio-bridge-backend") << " [options]" << std::endl << std::endl
      << "Options:" << std::endl
      << "--help, -h" << std::endl
      << "\tPrint this help message." << std::endl
      << "--daemonize, -B" << std::endl
      << "\tRun this program as a service." << std::endl
      << "--user, -u <username>" << std::endl
      << "\tRun this program as another user. (default: nobody)" << std::endl
      << "--group, -g <groupname>" << std::endl
      << "\tRun this program as another group. (default: nobody)" << std::endl
      << "--dbpath, -d <filename>" << std::endl
      << "\tSet the path of the DB file to use." << std::endl
      << "--socket, -s <ip4:socket>" << std::endl
      << "\tSet the listening socket. (default: 0.0.0.0:8080)" << std::endl;
      
  std::exit(0);
}

cArg::cArg(int argc, char **argv)
{
  m_isDaemon = false;
  m_user  = "nobody";
  m_group = "nobody";
  
  m_ip    = "0.0.0.0";
  m_port  = 8080;
  
  m_db_path.clear();
  
  _parse(argc, argv);
}

cArg::~cArg()
{
}

void cArg::_parse(int argc, char **argv)
{
  int option_index = 0;
  int c;
  
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
        m_isDaemon = true;
        break;
      
      case 'u':
        m_user = optarg;
        break;
      
      case 'g':
        m_group = optarg;
        break;
      
      case 'd':
        m_db_path = optarg;
        break;
      
      case 's':
      {
        std::string tmp(optarg); std::string::size_type pos = tmp.find(':');
        if( pos == std::string::npos )
        {
          std::cerr << "[-] Invalid socket format" << std::endl;
          printHelp(std::cerr, argv[0]);
        }
        m_ip    = tmp.substr(0, pos);
        m_port  = std::stoi( tmp.substr(pos +1) );
        break;
      }
      
      default:
        printHelp(std::cerr, argv[0]);
        break;
    }
  }
    
}

const bool cArg::daemonize() const noexcept
{
  return m_isDaemon;
}

const std::string& cArg::getUserName() const noexcept
{
  return m_user;
}

const std::string& cArg::getGroupName() const noexcept
{
  return m_group;
}

const std::string& cArg::getDBPath() const noexcept
{
  return m_db_path;
}

const int cArg::getListeningPort() const noexcept
{
  return m_port;
}

const std::string& cArg::getListenIP() const noexcept
{
  return m_ip;
}
