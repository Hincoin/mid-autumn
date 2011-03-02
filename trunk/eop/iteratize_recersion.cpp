#include <iostream>
#include <cstdlib>
#include <cstdio>

float simple_rand()
{
    return rand() / (float)RAND_MAX;
}

float rt(float r);
float s_r(float x,float&);
float s_t(float y,float&);

static const int max_depth=6;
static float test_rand_data[max_depth];

float rt(float r,int depth)
{
    if(r == 0.f) return 0.f;
    float new_r,new_t;
    if(depth++ < max_depth)
    {
        printf("rt_depth: %d \t",depth-1);
        float t_sr = s_r(r,new_r);
        float t_st = s_t(r,new_t);
        return test_rand_data[depth-1]+t_sr * rt(new_r,depth) + t_st * rt(new_t,depth);
    }
    return 1;
}

float s_r(float x,float &out)
{
    out = x+x;
    return ( int(x * 1000) % max_depth) ?   (x*x)  : 0.f ;
}
float s_t(float y,float &out)
{
    out = y*y;
    return ( int(y * 1000) % max_depth) ? (y+y) : 0.f ;
}

float rt_non_recursive(float r)
{
    float float_stack[max_depth];
    float pt_stack[max_depth];
    bool left_stack[max_depth];
    int stack_top=0;
    float_stack[stack_top]=r;
    pt_stack[stack_top] = 1.f;
    left_stack[stack_top] = true;
    float result=0.f;
    stack_top++;
    enum{pre,in,post} visit=pre;
    
    while(stack_top > 0)
    {
        float l = test_rand_data[stack_top-1]; 
        float cur_r = float_stack[stack_top-1];
        result += l * pt_stack[stack_top-1];
        if(stack_top < max_depth && visit != post)
        {
            if(visit == pre)
            {
                pt_stack[stack_top] = s_r(cur_r,float_stack[stack_top]);
                if(pt_stack[stack_top] == 0.f)
                {
                    pt_stack[stack_top] = s_t(cur_r,float_stack[stack_top]);
                    if(pt_stack[stack_top] == 0.f)
                    {
                        stack_top --;    
                        visit = post;
                    }
                    else
                    {
                        printf("rtnon_rec_depth: %d \t",stack_top-1);
                        pt_stack[stack_top] *= pt_stack[stack_top-1];
                        left_stack[stack_top] = false;
                        stack_top ++;
                        visit = pre;
                        //mul
                    }
                }
                else
                {
                    printf("rtnon_rec_depth: %d \t",stack_top-1);
                    pt_stack[stack_top] *= pt_stack[stack_top-1];
                    left_stack[stack_top] = true;
                    visit = pre;
                    stack_top++;
                    //mul
                }
            }
            else if(visit == in)
            {
                pt_stack[stack_top] = s_t(cur_r,float_stack[stack_top]);
                if(pt_stack[stack_top] == 0.f)
                {
                    visit = post;
                    stack_top--;
                }
                else
                {
                    printf("rtnon_rec_depth: %d \t",stack_top-1);
                    pt_stack[stack_top] *= pt_stack[stack_top-1];
                    left_stack[stack_top] = false;
                    visit = pre;
                    stack_top++;
                    //mul
                }
            }
        }
        else
        {
            if(stack_top == max_depth)
            {
                if(visit == post) visit = in;
                else if(visit == pre) visit = in;
            }
            if(visit == in)visit = post;
            else if(visit == post && left_stack[stack_top-1]) visit = in;
            else if(visit == pre) visit = in;
 
            stack_top--;
       }
    }
    return result;
}
int main()
{
    srand(0);
    for(int i = 0;i < max_depth; ++i)
    {
        test_rand_data[i] = simple_rand();
    }

    std::cout<<rt(0.40f,0)<<std::endl;
    std::cout<<rt_non_recursive(0.4f)<<std::endl;
}
