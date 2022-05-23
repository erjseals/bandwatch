#ifndef _COMBINE_CLASS_H_
#define _COMBINE_CLASS_H_

#include <iostream>
#include <iterator>
#include <vector>
#include <cstdlib>

template <typename T> 
class Combine
{
    
public:


    // initialize status
   Combine(int32_t N, int32_t R, std::vector<T> *invector) :
       completed(N < 1 || R > N),
       generated(0),
       N(N), R(R)
   {
       vector_data = invector;

       for (uint32_t c = 1; c <= R; ++c){
           vector_index.push_back(c);
           //std::cout << "vi " << c << std::endl;
       }
           

   }

   // true while there are more solutions
   bool completed;

   // count how many generated
   int generated;


    std::vector<T> next(){
        std::vector<int> temp = _next();
        std::vector<T> out;
        for(size_t i=0; i<temp.size(); i++){
            //std::cout << temp.at(i) << std::endl;
            out.push_back(vector_data->at(temp.at(i)-1));
        }
        return out;
    }
    
private:

    int32_t N, R;
    std::vector<T> *vector_data;
    std::vector<int> vector_index;

    // get current and compute next combination
    std::vector<int> _next()
    {
       std::vector<int> ret = vector_index;

       // find what to increment
       completed = true;
       for (int i = R - 1; i >= 0; --i)
           if (vector_index[i] < N - R + i + 1)
           {
               int j = vector_index[i] + 1;
               while (i <= R-1)
                   vector_index[i++] = j++;
               completed = false;
               ++generated;
               break;
           }

       return ret;
    }
  
};

#endif
