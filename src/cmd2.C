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

struct BaseCmd{
	typedef std::pair<std::string,BaseField&> pfield;
	typedef std::vector<pfield (*)(BaseCmd *)> pvec;
};

struct UserCmd : public BaseCmd{

	FLD(UserCmd,IntField,sd,11)
	FLD(UserCmd,IntField,sz,12)
	FLD(UserCmd,FlagField,off,false)
/*
	IntField sz;
	static BaseCmd::pfield szF(BaseCmd* p)
	{
		return BaseCmd::pfield("sz",static_cast<UserCmd*>(p)->sz);
	}
*/

	static BaseCmd::pvec info;

	static BaseCmd::pvec init()
	{
		BaseCmd::pvec tmp;
		tmp.push_back(&UserCmd::sdF);
		tmp.push_back(&UserCmd::szF);
		tmp.push_back(&UserCmd::offF);
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
		return ( size() > 1 ) ? (*p1 == '-' && *(p1+1) == '-') : false;
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
			if( *pos == ' ' )
			{
				if( p != ' ' )
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

		if( tokens.size() && !tokens.back().p2)
		{
			auto& b = tokens.back();

			b.p2 = pos;

			std::cout 
				<< " s=" << b.size()
				<< " b=" << b.as_string()
				<< '\n'  << std::endl;
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
		ucmd.init();
		adminMgr.reg(" tail mail", ucmd.info, ucmd);

    std::tuple t{}; // Another C++17 feature: class template argument deduction
    std::apply([](auto&&... args) {((std::cout << args << '\n'), ...);}, t);

		std::cout << std::get<1>(myT.f).v << '\n' << std::endl;

    return 0;
}
