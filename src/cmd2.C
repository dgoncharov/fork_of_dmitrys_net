#include <vector>
#include <array>
#include <map>
#include <string>
#include <tuple>
#include <iostream>
#include <type_traits>
#include <functional>
#include <stdlib.h>

struct BaseField{
	virtual bool assign(const std::string& v_)=0;
};

struct IntField : public BaseField{
	long long v = 0;
	IntField(int v_) : v(v_) {}
	IntField(){}
	virtual bool assign(const std::string& v_)
	{
		v = atoll(v_.data());
		return v || v_ == "0";
	}
};

struct FltField : public BaseField{
	double v = 0;
	FltField(double v_) : v(v_) {}
	FltField(){}
	virtual bool assign(const std::string& v_)
	{
		v = atof(v_.data());
		return v!=0.0 || v_ == "0" || v_ == "0.0";
	}
};


struct StrField : public BaseField{
	std::string v;
	StrField(const std::string& v_) : v(v_) {}
	StrField(){}
	virtual bool assign(const std::string& v_)
	{
		v = v_;
		return true;
	}
};


struct FlagField : public BaseField{
	bool v = false;
	FlagField(bool v_) : v(v_) {}
	FlagField(){}
	virtual bool assign(const std::string& v_)
	{
		bool v1 = v_ == "on";	
		bool p1 = v_ == "off";	
		v = v1;
		return v1 || p1;
	}
};


#define GET_PARENT(identifier) (&std::remove_pointer_t<decltype(this)>::identifier)

#define FLDD(C,T,N,D,P) T N{D}; static BaseCmd::pfield N##F(BaseCmd* p) { return {#N,P,static_cast<C*>(p)->N};}
#define FLDN(C,T,N,P)      T N; static BaseCmd::pfield N##F(BaseCmd* p) { return {#N,P,static_cast<C*>(p)->N};}
/*
	IntField sz;
	static BaseCmd::pfield szF(BaseCmd* p)
	{
		return BaseCmd::pfield("sz",static_cast<UserCmd*>(p)->sz);
	}
*/

struct BaseCmd{

	struct FieldInfo{
		std::string name;
		bool        is_param;
		BaseField&  fld;
	};

	typedef FieldInfo pfield;
	typedef std::array<BaseCmd::pfield (*)(BaseCmd *),3> pvec;
};

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

	typedef std::function<void(const char*)> AdminFunc;
	std::map<std::string,AdminFunc> adminF;

	template <typename T>
	void reg(const std::string& name_, std::function<void(T&)> f)
	{
		adminF[name_] = [&](const char* cmdline) { 
			T t; 
			run(cmdline, t.init(), t);
			f(t); 
		};
	}

	void run(const std::string& name_, const char* cmd_)
	{
		adminF[name_](cmd_);	
	}

	void run(const char* cmd_, 	const BaseCmd::pvec pvec_, BaseCmd& b)
	{
		char p = ' ';
		char t = '\0'; //t can be (0 f(field) ' ")
		int  id = 0;
		bool e = false;

		std::vector<token> tokens;

		const char* pos = cmd_;

		while( *pos )
		{
			/*
			std::cout  
				<< " pos="<< pos
				<< " t=["  << t <<"]"
				<< " p=["  << p <<"]"
				<< " e="  << e
				<< '\n'  << std::endl;
			*/

			if( *pos == ' ' && t && t != 'f' && (p != t || e) )
			{
				if(*(pos+1) == '\0')
				{
					std::cout  << "unclosed e at:"  << (pos - cmd_) << '\n'  << std::endl;
					return;
				}
			}
			else if( *pos == ' ' || (t == 'f' && *(pos+1) == '\0') )
			{
				if ( *(pos+1) == '\0' && *pos != ' ' )
				{
					if(t && t != 'f' && t != *pos)
					{
						std::cout  << "unclosed f at:"  << (pos - cmd_) << '\n'  << std::endl;
						return;
					}
					if(tokens.empty() || tokens.back().p2)
					{
						std::cout  << "internal parsing error at ["  << pos << "] p2:" << tokens.back().p2 << '\n'  << std::endl;
						return;
					}
					tokens.back().p2 = pos+1;
				}
				else if( p != ' ' )
				{
					tokens.back().p2 = pos;
				}
				t = '\0';
			}
			else if( p == ' ' && t && t != 'f')
			{
			}
			else if( p == ' ' )
			{
				if( *pos == '\"' || *pos == '\'' )
					t=*pos;
				else
					t = 'f';

				if(!tokens.empty() && !tokens.back().p2)
				{
					std::cout  << "internal parsing error at ["  << pos << "] p1:" << tokens.back().p1 << '\n'  << std::endl;
					return;
				}

				tokens.push_back(token());
				tokens.back().p1 = pos;
				tokens.back().id = id++;

				if( *(pos+1) == '\0' )
				{
					tokens.back().p2 = pos+1;
				}
			}
			e = (!e && p == '\\');
			p = *pos;
			++pos;
		}

/*
		for(auto& t : tokens)
		{
			std::cout  
				<< " p1=" << t.p1 - cmd_
				<< " p2=" << t.p2 - cmd_
				<< " c =" << t.as_string()
				<< '\n'  << std::endl;
		}

*/
		
		for(auto& f : pvec_)
		{
			auto p = f(&b);
			auto idx = &f - &pvec_.front();
			if( idx < tokens.size() )
			{
				if (!p.fld.assign(tokens[idx].as_string()) )
				{
					std::cout  << "failed to assign param:" << p.name << " with value:"  << tokens[idx].as_string() << '\n'  << std::endl;
				}
			}
		}
	}

};

typedef std::tuple<
IntField,
IntField
> Fields;

struct UserCmd : public BaseCmd{

	FLDN(UserCmd,IntField,sz,false)
	FLDN(UserCmd,FltField,sd,false)
	FLDN(UserCmd,FlagField,off,false)

	static constexpr BaseCmd::pvec init()
	{
		return { &UserCmd::szF, &UserCmd::sdF, &UserCmd::offF };
	}
};

void cmdF(UserCmd& cmd)
{
		std::cout
			<< " Execution:"
			<< " sz="  << cmd.sz.v
			<< " sd="  << cmd.sd.v
			<< " off=" << cmd.off.v
			<< '\n'  << std::endl;
}
	
int main(int argc, const char * argv[]) {

		//not related sanbox with tuples
    //std::tuple t{};
    //std::apply([](auto&&... args) {((std::cout << args << '\n'), ...);}, t);
		//std::cout << std::get<1>(myT.f).v << '\n' << std::endl;

		if(argc != 2)
		{
			std::cout  << "pass quoted command with format: int str onoff ex: 12 str off" <<  '\n'  << std::endl;
			return 1;
		}
		else
		{
			std::cout  << "command:" << argv[1] << '\n' << std::endl;
		}
		
		AdminMgr adminMgr;

		adminMgr.reg<UserCmd>("tail", cmdF);

		adminMgr.run("tail", argv[1]);


    return 0;
}
