#include "mpi.h"
#include <iostream>
#include "stdlib.h"
using namespace std;

double fact(int n) { 
    if (n < 0) {
        std::cout << "Erorr: the factorial of a negative number" << std::endl;
        exit(1);
    }
    if (n == 0) {
        return 1;
    } else { 
        return n * fact(n - 1); 
    }
}

int main(int argc, char *argv[]) {

  int terms_n, process_n, id;

  long double result, local_exp = 0;
  long double time_bedin, time_end;
  
  terms_n = atoi(argv[1]);

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD,&process_n); 
  MPI_Comm_rank(MPI_COMM_WORLD,&id); 

  if (id == 0) { 
      time_bedin = MPI_Wtime();
  }

  MPI_Bcast(&terms_n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  for (int i = id; i <= terms_n; i += process_n) {
      local_exp += 1 / fact(i);
  }

  MPI_Reduce(&local_exp, &result, 1, MPI_LONG_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  cout.precision(20);
  if (id == 0) {   
      cout << "Result: " << result << endl; 
      time_end = MPI_Wtime();
      cout << "Time: "<<(time_end - time_bedin) * 1000 << endl;      
  }

  MPI_Finalize();
  return 0;
}
