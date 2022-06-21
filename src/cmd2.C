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

#include <sys/types.h>
#include <sys/stat.h>


struct BaseField{
	virtual void exec(const std::string v_)=0;
};


struct IntField : public BaseField{
	int v;
	IntField(int v_) : v(v_) {}
	virtual void exec(const std::string v_)
	{
	}
};

struct BaseCmd{
	typedef std::pair<std::string,BaseField&> pfield;
	typedef std::vector<pfield (*)(BaseCmd *)> pvec;
};

struct UserCmd : public BaseCmd{

	IntField sz;
	IntField sd;

	UserCmd()
	: sz(1)
	,	sd(2)
	{}

	static BaseCmd::pfield sdF(BaseCmd* p)
	{
		return BaseCmd::pfield("sd",static_cast<UserCmd*>(p)->sd);
	}

	static BaseCmd::pfield szF(BaseCmd* p)
	{
		return BaseCmd::pfield("sz",static_cast<UserCmd*>(p)->sz);
	}

	static BaseCmd::pvec info;

	static BaseCmd::pvec init()
	{
		BaseCmd::pvec tmp;
		tmp.push_back(&UserCmd::szF);
		tmp.push_back(&UserCmd::sdF);
		return tmp;
	}

} ucmd;

BaseCmd::pvec UserCmd::info(UserCmd::init());

struct AdminMgr{
	void reg(std::string id, 	BaseCmd::pvec& pvec_, BaseCmd& b)
	{
		for(auto f : pvec_)
		{
			auto p = f(&b);
    	printf("field:%d\n", static_cast<IntField&>(p.second).v);
		}
	}
};

int main(int argc, const char * argv[]) {
		
		AdminMgr adminMgr;
		ucmd.init();
		adminMgr.reg("tail", ucmd.info, ucmd);

    return 0;
}
