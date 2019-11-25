#include <string>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <getopt.h>

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Tooling/ASTDiff/ASTDiff.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

static bool isHelp = false;
static bool isVerbose = false;

std::unique_ptr<CompilerInstance> getCompilerInstance(std::string &fileName)
{
	std::unique_ptr<CompilerInstance> Ci(new CompilerInstance);
	Ci->createDiagnostics(nullptr,false);

	std::shared_ptr<clang::TargetOptions> ptrTaropt(new clang::TargetOptions);
	ptrTaropt->Triple = sys::getDefaultTargetTriple();
	TargetInfo *ptrTarInfo = TargetInfo::CreateTargetInfo(Ci->getDiagnostics(),ptrTaropt);
	Ci->setTarget(ptrTarInfo);

	Ci->getHeaderSearchOpts().AddPath("/usr/local/include", clang::frontend::Angled, false, false);
	Ci->getHeaderSearchOpts().AddPath("/usr/lib/llvm-6.0/lib/clang/6.0.0/include", clang::frontend::Angled, false, false);
	Ci->getHeaderSearchOpts().AddPath("/usr/include/x86_64-linux-gnu", clang::frontend::Angled, false, false);
	Ci->getHeaderSearchOpts().AddPath("/usr/lib/llvm-6.0/include", clang::frontend::Angled, false, false);
	Ci->getHeaderSearchOpts().AddPath("/include", clang::frontend::Angled, false, false);
	Ci->getHeaderSearchOpts().AddPath("/usr/include", clang::frontend::Angled, false, false);
	Ci->getHeaderSearchOpts().AddPath("/usr/include/llvm-6.0", clang::frontend::Angled, false, false);

	Ci->createFileManager();
	Ci->createSourceManager(Ci->getFileManager());
	Ci->createPreprocessor(TU_Complete);
	Ci->createASTContext();

	const FileEntry *ptrFileEntry = Ci->getFileManager().getFile(fileName.c_str());
	Ci->getSourceManager().setMainFileID(Ci->getSourceManager().createFileID(ptrFileEntry,SourceLocation(),SrcMgr::C_User));
	
	ASTConsumer *Consumer = new ASTConsumer;
	Ci->setASTConsumer(std::unique_ptr<ASTConsumer>(Consumer));
	Ci->createSema(TU_Complete,nullptr);

	Ci->getDiagnostics().setSeverityForAll(clang::diag::Flavor::WarningOrError, clang::diag::Severity::Ignored);
	Ci->getDiagnosticClient().BeginSourceFile(Ci->getLangOpts(), &Ci->getPreprocessor());
	ParseAST(Ci->getPreprocessor(), &*Consumer, Ci->getASTContext());
	Ci->getDiagnosticClient().EndSourceFile();
	return Ci;
}

std::pair<std::string, std::string> parseCommandLine(int argc, char *argv[]) 
{
    int character = 0;
	int optIndex = 0;
	std::pair<std::string, std::string> commandPair;
	static struct option optList[] = {
	  {"help", no_argument, NULL, 'h'},
	  {"verbose", no_argument, NULL, 'v'}
	};

    while ((character = getopt_long(argc, argv, "hv", optList, &optIndex)) != -1) 
	{
	  switch (character) 
	  {
		case 'h': 
			isHelp = true;
			break;
		case 'v': 
			isVerbose = true;
			break;
		default: 
			printf("Unknown option: %c\n",(char)optopt); 
			break;
	  }
	}
    if (argc == optind + 2)  //two argv
	{
	  commandPair.first = std::string(argv[optind]); //first 
	  commandPair.second = std::string(argv[optind + 1]); //second
	}
	return commandPair;
}

int getTreeSize(diff::SyntaxTree &Tree)
{
	int size = 1;
	std::vector<diff::NodeId> nodeIdTree;

	nodeIdTree.push_back(Tree.getRootId());//add rootId
	while(!nodeIdTree.empty())
	{
		const diff::Node &node = Tree.getNode(nodeIdTree.back());
		nodeIdTree.pop_back();

		for(diff::NodeId id : node.Children)
		{
			nodeIdTree.push_back(id);//add Children
			size = size + 1;
		}
	}

	return size;
}

double calSimilarity(diff::ASTDiff &diffAST, diff::SyntaxTree &srcTree, 
											diff::SyntaxTree &dstTree) 
{
	double sim = 0.0;
	double editDist = 0.0;
	double total = 0.0;

	for (diff::NodeId nodeId_dst : dstTree)
	{
		const diff::Node &node  = dstTree.getNode(nodeId_dst);
		switch (node.Change)
		{
		case diff::None:
			break;
		case diff::Delete://(src): delete node src. the dstTree shuoldn't have deletions
			break;
		case diff::Update://(src,dst): update the value of node src to match dst;
			editDist += 1; 
			break;
		case diff::Insert://(src,dst,pos): insert src as child of dst at offset pos.
			editDist += 1; 
			break;
		case diff::Move://(src,dst,pos): move src to be a child of dst at offset pos. 
			editDist += 1; 
			break;
		case diff::UpdateMove: //same as move plus udpate
			editDist += 2;
			break;
		default:
			break;
		}
	}

	//also should consider the invalid node in src which can not be changed
	for(diff::NodeId nodeId_src : srcTree)
	{
		if(diffAST.getMapped(srcTree,nodeId_src).isInvalid())
			editDist += 1; 
	}


	total = getTreeSize(srcTree);
	total += getTreeSize(dstTree);
	sim = 1 - editDist/total;
	return sim;
}

int main(int argc, char *argv[]) 
{
    std::pair<std::string, std::string> commandPair;

	//parse command line to get two argvs
	commandPair = parseCommandLine(argc, argv);
    if (commandPair.first.empty()) 
	{
		printf("[usage: codesim [-v|--verbose](todo) [-h|--help] code1 code2\n");
		return 1;
	}
	else if(commandPair.second.empty())
	{
		
		if (isHelp)
		{
			printf("[usage: codesim [-v|--verbose](todo) [-h|--help] code1 code2\n");
		}
		else if(isVerbose)
		{
			printf("[usage: codesim [-v|--verbose](todo) [-h|--help] code1 code2\n");
		}
		else
		{
			//may first is code file path, second code file path is null, return 1
			printf("[usage: codesim [-v|--verbose](todo) [-h|--help] code1 code2\n");
			return 1;
		}
		
		return 0;
	}
	std::string srcFile = commandPair.first; //srcFile path
	std::string dstFile = commandPair.second; //dstFile path

	std::unique_ptr<CompilerInstance> srcCi = getCompilerInstance(srcFile);
	std::unique_ptr<CompilerInstance> dstCi = getCompilerInstance(dstFile);

	diff::ComparisonOptions opt; //top-down match or bottom-up match

	diff::SyntaxTree srcTree(srcCi->getASTContext());
	diff::SyntaxTree dstTree(dstCi->getASTContext());

	diff::ASTDiff diffAst(srcTree, dstTree, opt);
	//calculate the similarity of two syntax tree
	double sim = 0.0;
	sim = calSimilarity(diffAst, srcTree, dstTree);
	printf("%.2f\n", 100.0 * sim);
    return 0;
}
