#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"


int test_clone_var = 0;
void test_clone_fun() {
  test_clone_var = 5;
  exit();
}

void test_clone() {
  THREAD t;
  int args[1];
  args[0] = 7;
  t = pthread_create(test_clone_fun, (void*)args, CLONE_VM | CLONE_THREAD);
  pthread_join(t);
  if(test_clone_var == 5) {
    printf(1, "Clone Test : Pass\n\n");
  } else {
    printf(1, "Clone Test : Fail\n\n");
  }
  return;
}


int test_tkill_var = 0;
void test_tkill_fun() {
  sleep(100);
  test_tkill_var = 7;
  exit();
}

void test_tkill() {
  THREAD t;
  int args[1];
  args[0] = 7;
  t = pthread_create(test_tkill_fun, (void*)args, CLONE_VM | CLONE_THREAD);
  pthread_kill(t.tid);
  if(test_tkill_var == 0) {
    printf(1, "tkill Test : Pass\n\n");
  } else {
    printf(1, "tkill Test : Fail\n\n");
  }
  return;
}

int test_clone_in_thread_var = 8;
void test_clone_in_thread_fun2() {
  test_clone_in_thread_var += 1;
  exit();
}

void test_clone_in_thread_fun1() {
  THREAD t;
  int args[1];
  args[0] = 4;
  test_clone_in_thread_var += 1;
  t = pthread_create(test_clone_in_thread_fun2, (void*)args, CLONE_VM | CLONE_THREAD);
  pthread_join(t);
  test_clone_in_thread_var += 1;
  exit();
}

void test_clone_in_thread() {
  THREAD t;
  int args[1];
  args[0] = 4;
  t = pthread_create(test_clone_in_thread_fun1, (void*)args, CLONE_VM | CLONE_THREAD);
  pthread_join(t);
  if(test_clone_in_thread_var == 11 ) {
    printf(1, "clone in thread Test : Pass\n\n");
  } else {
    printf(1, "clone in thread Test : Fail\n\n");
  }
  return;
}


int no_of_threads = 3;
int r1,c1,r2,c2;
int mat1[100][100];
int mat2[100][100];
int res[100][100];

void test_mat_mul(void *arg) {
  int n = (int)arg;
  //printf("%d\n",r1);
  int start = (n*r1)/no_of_threads;
  int end = ((n+1)*r1)/no_of_threads;

  for(int i = start ; i < end; i++){
      for(int j = 0; j < c2; j++){
          res[i][j] = 0;
          for(int k = 0 ; k < c1; k++){
              res[i][j] += mat1[i][k]*mat2[k][j];
          }
      }
  }
  exit();
}



void test_id_fun(void* arg){
  int tid = gettid();
  int parent_id = getppid();
  int tgid = getpid();
  printf(1, "Thread ID : %d\nParent_ID : %d\nThread get ID : %d\n\n", tid,parent_id, tgid);
  exit();
}

void test_id(){
  printf(1,"Thread IDs\n");
  THREAD t1;
  t1 = pthread_create(test_id_fun, 0, CLONE_VM);
  pthread_join(t1);
} 




void test_multi_join_fun(void* flags) {
  int var = (int)flags;
  sleep(var);
  exit();
}

void test_multi_join() {
  THREAD t1, t2;
  int ret1, ret2;
  int var1 = 100, var2 = 200;
  t1 = pthread_create(test_multi_join_fun, (void*)var1, CLONE_VM | CLONE_THREAD);
  t2 = pthread_create(test_multi_join_fun, (void*)var2, CLONE_VM | CLONE_THREAD);
  ret1 = pthread_join(t1);
  ret2 = pthread_join(t2);
  if (ret1 == t1.tid && ret2 == t2.tid) {
    printf(1, "Test Join in order : Pass\n\n");
  } else {
    printf(1, "Test Join in order : Fail\n\n");\
  }
}


int test_fork_in_thread_var = 0;
void test_fork_in_thread_fun(void *args) {
  int ret = fork();
  if(ret == 0) {
    test_fork_in_thread_var += 5;
    exit();
  } else {
    wait();
    test_fork_in_thread_var += 10;
  }
  exit();
}

void test_fork_in_thread() {
  THREAD t;
  int var1 = 9;
  // printf()
  t = pthread_create(test_fork_in_thread_fun, (void*)&var1, CLONE_VM | CLONE_THREAD);
  pthread_join(t);
  if(test_fork_in_thread_var == 10) {
    printf(1, "Test fork in thread : Pass\n\n");
  } else {
    printf(1, "Test fork in thread : Fail\n\n");\
  }
}





int main() {
  test_clone();
  test_tkill();
  test_id();
  test_multi_join();
  test_fork_in_thread();
  test_clone_in_thread();
  exit();
}