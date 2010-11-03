#include "construct_by_connection.hpp"


//left,right,up,down
enum TestConnectorType{
    TUnknownConnector=0x0,
    TL=0x01,TR = TL << 1,TU = TR << 1,TD = TU << 1, 
    TUR = TU | TR, TRD = TR | TD, TDL = TD|TL, TLU = TL|TU , TLR = TL|TR, TUD = TU|TD, 
    TLUR = TL|TUR,TDLU = TD|TLU ,TRDL = TR|TDL,TURD=TU|TRD,
    TLURD = TLUR | TD
};

//for print
static const char* TestConnectTypeStr[] = 
{ 
    "UnknownOrCount",
    "L    ",//01
    "R    ",//02
    "LR   ",//03
    "U    ",//04
    "LU   ",//05
    "UR   ",//06
    "LUR  ",//07
    "D    ",//08
    "DL   ",//09
    "RD   ",//10
    "RDL  ",//11
    "UD   ",//12 
    "DLU  ",//13
    "URD  ",//14
    "LURD ",//15
};

std::vector<Connector*> init_connectors()
{
    std::vector<Connector*> ret;
    //Connector* l = new Connector(TL,PL);ret.push_back(l);
    //Connector* r = new Connector(TR,PR);ret.push_back(r);
    Connector* lr = new Connector(TLR,PL|PR);ret.push_back(lr);
    //Connector* u = new Connector(TU,PU);ret.push_back(u);
    Connector* lu = new Connector(TLU, PL|PU);ret.push_back(lu);
    Connector* ur = new Connector(TUR, PR|PU);ret.push_back(ur);
    Connector* lur = new Connector(TLUR, PL|PU|PR);ret.push_back(lur);
    //Connector* d = new Connector(TD, PD);ret.push_back(d);
    Connector* dl = new Connector(TDL, PD|PL);ret.push_back(dl);
    Connector* rd = new Connector(TRD, PR|PD);ret.push_back(rd);
    Connector* rdl = new Connector(TRDL, PR|PD|PL);ret.push_back(rdl);
    Connector* ud = new Connector(TUD, PU|PD);ret.push_back(ud);
    Connector* dlu = new Connector(TDLU, PD|PL|PU);ret.push_back(dlu);
    Connector* urd = new Connector(TURD, PU|PR|PD);ret.push_back(urd);
    //Connector* lurd = new Connector(TLURD, PL|PU|PR|PD);ret.push_back(lurd);

    std::vector<Connector*> ls,nls,us,nus,rs,nrs,ds,nds;
    ls = filter_by_key(TL,ret);nls = filter_by_not_key(TL,ret);
    us = filter_by_key(TU,ret);nus = filter_by_not_key(TU,ret);
    rs = filter_by_key(TR,ret);nrs = filter_by_not_key(TR,ret);
    ds = filter_by_key(TD,ret);nds = filter_by_not_key(TD,ret);
/*
    l->add_connector_1d<L>(rs);
    l->add_connector_1d<U>(nds);
    l->add_connector_1d<R>(nls);
    l->add_connector_1d<D>(nus);

    u->add_connector_1d<L>(nrs);
    u->add_connector_1d<U>(ds);
    u->add_connector_1d<R>(nls);
    u->add_connector_1d<D>(nus);


    r->add_connector_1d<L>(nrs);
    r->add_connector_1d<U>(nds);
    r->add_connector_1d<R>(ls);
    r->add_connector_1d<D>(nus);

    d->add_connector_1d<L>(nrs);
    d->add_connector_1d<U>(nds);
    d->add_connector_1d<R>(nls);
    d->add_connector_1d<D>(us);


    assert((!is_connector_match<L,R>(r,d)));
    */
    lr->add_connector_1d<L>(rs);
    lr->add_connector_1d<U>(nds);
    lr->add_connector_1d<R>(ls);
    lr->add_connector_1d<D>(nus);

    lu->add_connector_1d<L>(rs);
    lu->add_connector_1d<U>(ds);
    lu->add_connector_1d<R>(nls);
    lu->add_connector_1d<D>(nus);

    dl->add_connector_1d<L>(rs);
    dl->add_connector_1d<U>(nds);
    dl->add_connector_1d<R>(nls);
    dl->add_connector_1d<D>(us);

    ur->add_connector_1d<L>(nrs);
    ur->add_connector_1d<U>(ds);
    ur->add_connector_1d<R>(ls);
    ur->add_connector_1d<D>(nus);

    rd->add_connector_1d<L>(nrs);
    rd->add_connector_1d<U>(nds);
    rd->add_connector_1d<R>(ls);
    rd->add_connector_1d<D>(us);

    ud->add_connector_1d<L>(nrs);
    ud->add_connector_1d<U>(ds);
    ud->add_connector_1d<R>(nls);
    ud->add_connector_1d<D>(us);

    lur->add_connector_1d<L>(rs);
    lur->add_connector_1d<U>(ds);
    lur->add_connector_1d<R>(ls);
    lur->add_connector_1d<D>(nus);

    rdl->add_connector_1d<L>(rs);
    rdl->add_connector_1d<U>(nds);
    rdl->add_connector_1d<R>(ls);
    rdl->add_connector_1d<D>(us);

    dlu->add_connector_1d<L>(rs);
    dlu->add_connector_1d<U>(ds);
    dlu->add_connector_1d<R>(nls);
    dlu->add_connector_1d<D>(us);

    urd->add_connector_1d<L>(nrs);
    urd->add_connector_1d<U>(ds);
    urd->add_connector_1d<R>(ls);
    urd->add_connector_1d<D>(us);

   // lurd->add_connector_1d<L>(rs);
   // lurd->add_connector_1d<U>(ds);
   // lurd->add_connector_1d<R>(ls);
   // lurd->add_connector_1d<D>(us);

    return ret;
}

#include <iostream>
#include <cstdlib>
#include <cstdio>

void test_case()
{
    std::vector<Connector*> cs = init_connectors();
    for(int i = 2;i < 20; ++i)
        for(int j = 2;j < 20; ++j)
        {
            ConnectorMatrix m = construct_by_connection(i,j,i*20 + j,cs);
            assert(is_solution(m));
            if(is_solution(m))
            {   //output 
                for(size_t z = 0; z < m.size(); ++z)
                {
                    for(size_t x = 0; x < m[z].size(); ++x)
                    {
                        printf("%s",TestConnectTypeStr[m[z][x]->get_key()]);
                    }
                    printf("\n");
                }
                printf("\n");
            }
        }

}
int main()
{
    test_case();
    return 0;
}
