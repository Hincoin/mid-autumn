
void back_tracking(Status cur_status)
{
    //
    while(possible(cur_status, condition))
    {
        back_tracking_recursive(cur_status);
        step(cur_status, condition);
    }
}

struct back_tracking{
    void operator()()const
    {
    }
};

typedef int ConnectorType;
enum DirectionType{L=1, U = L << 1, R = U << 1, D = R << 1};
template<size_t N>
struct log2_n
{
    enum{value = 1 + log2_n< N >> 1>::value};
};
template<>
struct log2_n<1>
{
    enum{value = 0};
}
template<>
struct log2_n<0>
{
    enum{value = 0};
}
class Connector{
    typedef std::vector<Connector*> element_array;
    element_array connectors[4];
    ConnectorType key_;//identifier of this element
    public:
    explicit candidates(ConnectorType key):key_(key){}
    template<int N>
    const std::vector<Connector*>& get_connector()const
    {
        return connectors[log2_n<N>::value];
    }
    ConnectorType get_key()const{return key_;};
};
template<int D>
std::vector<Connector*> filter_by(const Connector& c)
{
    //return 
}
int main()
{
    return 0;
}
