#include <iostream>
#include <vector>
#include <queue>
#include <omp.h>
#include <algorithm>
#include "graph.h"

int main(int args, char **argv)
{
	std::cout<<"Input: ./exe beg csr source\n";
	if(args!=4){std::cout<<"Wrong input\n"; return -1;}
	
	const char *beg_file=argv[1];
	const char *csr_file=argv[2];
	long source = std::stol(argv[3]);

	//template <file_vertex_t, file_index_t, file_weight_t
	//new_vertex_t, new_index_t, new_weight_t>
	graph<long, long, /*int*/long, long, long,/* char*/long>
	*ginst = new graph
	<long, long, /*int*/long, long, long, /*char*/long>
	(beg_file,csr_file);

    double tm=wtime();

    std::vector<long> levels(ginst->vert_count, -1);
    levels[source] = 0;

    std::queue<long> global_queue;
    global_queue.push(source);

    while (!global_queue.empty()){
        long current_size = global_queue.size();

        #pragma omp parallel
        {
            std::queue<long> local_queue;

            #pragma omp for nowait
            for (long i = 0; i < current_size; ++i){
                long current;
                #pragma omp critical
                {
                    current = global_queue.front();
                    global_queue.pop();
                }

                for (long j = ginst->beg_pos[current]; j < ginst->beg_pos[current+1]; ++j){
                    long neighbor = ginst->csr[j];
                    if (__sync_bool_compare_and_swap(&levels[neighbor], -1, levels[current] + 1)){
                        local_queue.push(neighbor);
                    }
                }
            }

            #pragma omp critical
            {   
                while (!local_queue.empty()){
                    global_queue.push(local_queue.front());
                    local_queue.pop();
                }
            }
        }
    }

    /* single-threaded version for testing */

    // std::vector<long> levels(ginst->vert_count, -1);
    // std::queue<long> work_queue;

    // levels[source] = 0;
    // work_queue.push(source);

    // while (!work_queue.empty()) {
    //     long current = work_queue.front();
    //     work_queue.pop();

    //     for (long i = ginst->beg_pos[current]; i < ginst->beg_pos[current+1]; i++) {
    //         long neighbor = ginst->csr[i];
    //         if (levels[neighbor] < 0) {
    //             levels[neighbor] = levels[current] + 1;
    //             work_queue.push(neighbor);
    //         }
    //     }
    // }

    tm = wtime() - tm;
    auto max_iterator = std::max_element(levels.begin(), levels.end());
    long max_level = *max_iterator;
    long max_vertex = std::distance(levels.begin(), max_iterator);
    
    std::cout << "=========================" << std::endl;
    std::cout << "BFS starting from vertex " << source << " is finished!" << std::endl;
    std::cout << "Vertex " << max_vertex << 
         " has max level " << max_level << " (might not be unique)!" << std::endl;
    std::cout << "Traversal time not including loading is " << tm <<" second(s)." << std::endl;
	
    return 0;	
}
