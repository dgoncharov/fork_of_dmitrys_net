#include <sys/socket.h>
#include <sys/un.h>
#include <sys/event.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <type_traits>

#include <sys/types.h>
#include <sys/stat.h>


struct BaseField{
	virtual void exec(const std::string v_)=0;
};


struct IntField : public BaseField{
	int v = 10;
	IntField(int v_) : v(v_) {}
	IntField() {}
	virtual void exec(const std::string v_)
	{
	}
};

struct FlagField : public BaseField{
	bool v = true;
	FlagField(bool v_) : v(v_) {}
	FlagField() {}
	virtual void exec(const std::string v_)
	{
	}
};


#define GET_PARENT(identifier) (&std::remove_pointer_t<decltype(this)>::identifier)

#define FLD(C,T,N,D) T N = D; static BaseCmd::pfield N##F(BaseCmd* p) { return BaseCmd::pfield(#N,static_cast<C*>(p)->N);}
/*
	IntField sz;
	static BaseCmd::pfield szF(BaseCmd* p)
	{
		return BaseCmd::pfield("sz",static_cast<UserCmd*>(p)->sz);
	}
*/

#define FLD2(C,N) info.push_back(C::N##F);

struct BaseCmd{
	typedef std::pair<std::string,BaseField&> pfield;
	typedef std::vector<pfield (*)(BaseCmd *)> pvec;
};

struct UserCmd : public BaseCmd{

	FLD(UserCmd,IntField,sd,11)
	FLD(UserCmd,IntField,sz,12)
	FLD(UserCmd,FlagField,off,false)

	static BaseCmd::pvec info;

	static BaseCmd::pvec init()
	{
		BaseCmd::pvec tmp;
		FLD2(UserCmd,sd)
		FLD2(UserCmd,sz)
		FLD2(UserCmd,off)
		return tmp;
	}
} ucmd;

BaseCmd::pvec UserCmd::info(UserCmd::init());

struct token
{
	char  t='\0';
	const char* p1=0;
	const char* p2=0;
	int   id=0;

	std::string as_string() 
	{
		return std::string(p1,size());
	}
	size_t size() 
	{
		return p2 - p1;
	}
	size_t has_prefix() 
	{
		return ( size() > 1 ) ? (p1[0] == '-' && p1[1] == '-') : false;
	}
};

struct AdminMgr{
	void reg(const char* cmd_, 	BaseCmd::pvec& pvec_, BaseCmd& b)
	{
		char p = ' ';
		char t = '\0';
		int  id = 0;

		std::vector<token> tokens;

		const char* pos = cmd_;

		while( *pos )
		{
			if( *pos == ' ' || *(pos+1) == '\0' )
			{
				if ( *(pos+1) == '\0' && *pos != ' ' )
				{
					tokens.back().p2 = pos+1;
				}
				else if( p != ' ' )
				{
					tokens.back().p2 = pos;
				}

			}
			else if( p == ' ' )
			{
				tokens.push_back(token());
				tokens.back().p1 = pos;
				tokens.back().id = id++;
			}
			p = *pos;
			++pos;
		}

		for(auto& f : pvec_)
		{
			auto p = f(&b);
		
			auto idx = &f - &pvec_.front();

			if( idx < tokens.size() )
			{
				std::cout << p.first 
					//<< " v=" << static_cast<IntField&>(p.second).v
					<< " v=" << tokens[idx].as_string()
			 		<< '\n'  << std::endl;
			}
		}
	}

};

typedef std::tuple<
IntField,
IntField
> Fields;

struct MyT
{
	Fields f;
} myT;

int main(int argc, const char * argv[]) {
		
		AdminMgr adminMgr;
		adminMgr.reg(" tail mail", ucmd.info, ucmd);

    std::tuple t{};
    std::apply([](auto&&... args) {((std::cout << args << '\n'), ...);}, t);

		std::cout << std::get<1>(myT.f).v << '\n' << std::endl;

    return 0;
}
