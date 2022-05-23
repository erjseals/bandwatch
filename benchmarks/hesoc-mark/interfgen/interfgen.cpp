#include <sstream>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <sys/stat.h>
#include <bits/stdc++.h>
#include <set>
#include "datatypes.h"
#include "rapidxml/rapidxml.hpp"
#include "Combine.hpp"

const std::string filenameTag =  std::string("__THIS_FILENAME__");

std::string _filePrefix;
std::string _postProcStr;
std::string _preProcStr;
int32_t _sleepFor;

std::vector<int> getCoreIntVector(std::stringstream &ss){
    std::vector<int> result;
    while( ss.good() )
    {
        std::string substr;
        std::getline( ss, substr, ',' );
        result.push_back(std::stoi(substr));
    }
    return result;
}

std::string trim(const std::string &str){
    size_t s = str.find_first_not_of(" \n\r\t");
    size_t e = str.find_last_not_of (" \n\r\t");

    if(( std::string::npos == s) || ( std::string::npos == e))
        return "";
    else
        return str.substr(s, e-s+1);
}

void replaceStr(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return;
    str.replace(start_pos, from.length(), to);
}

std::string fromCmdToProcName(const std::string &cmdline){
    size_t found = cmdline.find_last_of("/\\");
    std::string out = trim(cmdline.substr(found+1));
    return out;
}

int32_t generateStructureFromXML(const std::string &inFile, testitem_t *under_test, std::vector<testitem_t> *vIFs, const bool verbosity){

rapidxml::xml_document<> doc;
rapidxml::xml_node<> * root_node;

//std::cout << "Read file" << std::endl;

// Read the xml file into a vector
std::ifstream theFile (inFile);
std::vector<char> buffer((std::istreambuf_iterator<char>(theFile)), std::istreambuf_iterator<char>());
buffer.push_back('\0');

// Parse the buffer using the xml file parsing library into doc 
doc.parse<0>(&buffer[0]);
// Find the root node
root_node = doc.first_node("test");

//iterations
const int32_t iters =  std::stoi(root_node->first_node("iterations")->value());

//TODO: check if root exist first!
_filePrefix += root_node->first_node("id")->value();

if(root_node->first_node("postprocstr")!=nullptr){
    _postProcStr = root_node->first_node("postprocstr")->value();
    //remove \n
    _postProcStr.erase(std::remove(_postProcStr.begin(), _postProcStr.end(), '\n'), _postProcStr.end());
} else _postProcStr = "";

//chrt -f 99 example
if(root_node->first_node("preprocstr")!=nullptr){
    _preProcStr = root_node->first_node("preprocstr")->value();
    //remove \n
    _preProcStr.erase(std::remove(_preProcStr.begin(), _preProcStr.end(), '\n'), _preProcStr.end());
} else _preProcStr = " ";

//check for sleepyness
if(root_node->first_node("sleep_s")!=nullptr)
	_sleepFor = atoi(root_node->first_node("sleep_s")->value());
else _sleepFor = 1;


if(verbosity){
    std::cout << "File prefix is " << _filePrefix << std::endl;
    std::cout << "Post process string is " << _postProcStr << std::endl;
}

//UTs
for (rapidxml::xml_node<> * test_node = root_node->first_node("UT"); test_node; test_node = test_node->next_sibling())
{

    under_test->procType = ProcType::UT;
    under_test->cmdLine = test_node->first_attribute("cmdLine")->value();
    under_test->argsLine = test_node->first_attribute("args")->value();
    under_test->cores = test_node->first_attribute("core")->value();
    under_test->engine = test_node->first_attribute("engine")->value();

    if(verbosity){
        std::cout << "Under test found : " << std::endl;
        std::cout << "  Command Line " << under_test->cmdLine << std::endl;
        std::cout << "  with args string " <<  under_test->argsLine << std::endl;
        std::cout << "  Replicated in cores " << under_test->cores << std::endl;
    }

    break; //We assume only one under test (as of now)
}

//IFs
for (rapidxml::xml_node<> * test_node = root_node->first_node("IF"); test_node; test_node = test_node->next_sibling())
{

    testitem_t interf;
    interf.procType = ProcType::IF;
    interf.cmdLine = test_node->first_attribute("cmdLine")->value();
    interf.argsLine = test_node->first_attribute("args")->value();
    interf.cores = test_node->first_attribute("core")->value();
    interf.engine = test_node->first_attribute("engine")->value();

    if(verbosity){
        std::cout << "Interferring app found : " << std::endl;
        std::cout << "  Command Line " << interf.cmdLine << std::endl;
        std::cout << "  with args string " << interf.argsLine << std::endl;
        std::cout << "  Replicated in cores " << interf.cores << std::endl;
    }

    vIFs->push_back(interf);

}

return iters;

}


inline bool parseArgs(interfgenArgs& args, int argc, char* argv[])
{
    while (1)
    {
        int arg;
        static struct option long_options[] = {{"help", no_argument, 0, 'h'},
            {"verbose", no_argument, 0, 'v'},
            {"outdir", required_argument, 0, 'o'},
            {"infile", required_argument, 0, 'f'},
            {nullptr, 0, nullptr, 0}};
        int option_index = 0;
        arg = getopt_long(argc, argv, "hvf:o:", long_options, &option_index);
	if (arg == -1)
        {
            break;
        }

        switch (arg)
        {
        case 'h': args.help = true; 
        case 'v': args.verbosity = true; break;
        case 'f':
            if (optarg)
            {
                args.infile = std::string(optarg);
            }
            break;
        case 'o':
            if (optarg)
            {
                args.outdir = std::string(optarg);
		        if(args.outdir.back()!='/') //checks for well formatted directory paths
		            args.outdir += "/";
            }
            break;
        default: return false;
        }
    }
    return true;
}


void showHelp(){

    std::cout << "Usage: ./interfgen [-h or --help] [-v or --verbose] [-f or --infile=<string>.xml] [-o or --outdir=<string>]" << std::endl;
    std::cout << "--help    Display help information" << std::endl;
    std::cout << "--verbose Activate verbose parser and .sh script generation (for debug and test)" << std::endl;
    std::cout << "--infile=<string>.xml     Specify the input XML file for the test configuration." << std::endl;
    std::cout << "--outdir=<string>         Specify the output directory to put the generated .sh scripts." << std::endl;

    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]){

interfgenArgs args;
args.verbosity = false;
args.outdir = "./";
args.help=false;


if(argc<=1){
 std::cout << "Not enough arguments. Please, have a look at the following help message..." << std::endl;
 showHelp();
}

parseArgs(args,argc,argv);

if(args.help)
 showHelp();

//Creates the output directory if it does not exist
struct stat sb;
if(!(stat(args.outdir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
   if(mkdir(args.outdir.c_str(),0700)!=0){
	std::cout << "Error in creating folder " << args.outdir << std::endl;
	exit(-1);
   }
}

bool verbosity = args.verbosity;
testitem_t under_test;
std::vector<testitem_t> vIFs;
std::set<std::string> killAllprocNames;

_filePrefix = args.outdir;

const int32_t iters = generateStructureFromXML(args.infile, &under_test, &vIFs, verbosity);

const uint32_t k = vIFs.size();

_filePrefix += "-" + under_test.engine + "_";

const std::string linuxSchedStr = _preProcStr; //was " chrt -f 99 ";

for(uint32_t i=1; i<=k; i++){
    Combine<testitem_t> c(k, i, &vIFs);

    while(!c.completed){

        int32_t count = 0;
        std::vector<testitem_t> v = c.next();

        std::string postProcStr = _postProcStr;
        std::string script = "#! /usr/bin/env bash \n\nsysctl -w kernel.sched_rt_runtime_us=-1 \n\n";
        script += "for i in {1.."+ std::to_string(iters) +"} \ndo \n";
        std::string toAppendToFilePrefix = "";

        for(uint32_t i = 0; i<v.size(); i++){

            std::string procName = fromCmdToProcName(v.at(i).cmdLine);
            killAllprocNames.insert(procName);

            std::stringstream ss = std::stringstream(v.at(i).cores);
            std::vector<int> iCores = getCoreIntVector(ss);
            if(iCores.size()==1){
                std::string script_str = "sudo taskset -c "; //set core affinity
                script_str += v.at(i).cores + " "; //linuxSchedStr; //set FIFO 99 priority. TODO, fix "multi core affinity"
                script_str += v.at(i).cmdLine + " " + v.at(i).argsLine + " & PID_TO_KILL" + std::to_string(count) + "=$!"; //launch app and its arguments
                script += script_str + "\n";
                toAppendToFilePrefix += v.at(i).engine + "_";
                count++;
            } else {
               for(uint32_t j=0; j<iCores.size(); j++){
                    std::string script_str = "sudo taskset -c "; //set core affinity
                    script_str += std::to_string(iCores.at(j)) +  " "; //linuxSchedStr; //set FIFO 99 priority. TODO, fix "multi core affinity"
                    script_str += v.at(i).cmdLine + " " + v.at(i).argsLine + " & PID_TO_KILL" + std::to_string(count) + "=$!"; //launch app and its arguments
                    script += script_str + "\n";
                    toAppendToFilePrefix += v.at(i).engine + "_";
                    count++;
               }
            }
        }

    std::string outfile = _filePrefix + toAppendToFilePrefix + ".sh";
    //be sure there are no "\n" in output file.
    outfile.erase(std::remove(outfile.begin(), outfile.end(), '\n'), outfile.end());

    size_t str_index = outfile.find_last_of("/");

    if(str_index==std::string::npos)
      str_index = 0;
    else str_index++;

    std::string sanitizedOutfile = outfile.substr(str_index);

    if (postProcStr.find(filenameTag) != std::string::npos) {
        const std::string filenameFinalPart = " " + sanitizedOutfile + "_$i" + ".txt"; 
        replaceStr(postProcStr,filenameTag,filenameFinalPart);
    }

    script += "sleep "+ std::to_string(_sleepFor)  +" \n";
	script += "sudo taskset -c " + under_test.cores + " " + linuxSchedStr + " " + under_test.cmdLine + " " + under_test.argsLine + " " + 
               postProcStr + " &  PID_TO_WAIT=$! \n";
	script += "wait $PID_TO_WAIT\n\n";
	script += "echo \"done\"\n";

    for(int32_t i=0; i<count; i++)
        script += "sudo kill -9 $PID_TO_KILL" + std::to_string(i) + " \n";

    std::set<std::string>::iterator it = killAllprocNames.begin();

    while (it != killAllprocNames.end()){
        script += "sudo killall -q " + (*it) + "\n";
	    it++;
    }

    script += "\nwait \ndone \n";

    if(verbosity){
        std::cout << "I am going to execute : " << std::endl;
        std::cout << script << std::endl;
        std::cout << "Will write file " << outfile << std::endl;
    }

    std::ofstream out(outfile);
    out << script;
    out.close();
    if(system( std::string("sudo chmod a+x " + outfile).c_str() )!=0)
    	std::cout << "Error in setting chmod options for " << outfile << std::endl;

    }

}


return EXIT_SUCCESS;

}
