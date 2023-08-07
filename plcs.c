#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "thread.h"
#include "thread-sync.h"
//思路 ：使用链表来维护一个队列，线程负责去链表中去取数据来执行
#define MAXN 10000
int T, N, M;
char A[MAXN + 1], B[MAXN + 1];
int used[MAXN + 1][MAXN+1];
int dp[MAXN][MAXN];
int result;
int count;

sem_t sem2,sem3;
typedef struct listnode
{
  /* data */
  int i;
  int j;
  struct listnode* next;
}m_list;
m_list* head=(m_list*)malloc(sizeof(m_list));
m_list* tmp_head=head;

#define DP(x, y) (((x) >= 0 && (y) >= 0) ? dp[x][y] : 0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MAX3(x, y, z) MAX(MAX(x, y), z)

void Tworker(int id) {
      while(count<M*N)
      {
        P(sem2);
      int i=head->next->i;
      int j=head->next->j;
      int skip_a = DP(i - 1, j);
      int skip_b = DP(i, j - 1);
      int take_both = DP(i - 1, j - 1) + (A[i] == B[j]);
      dp[i][j] = MAX3(skip_a, skip_b, take_both);
      if(i==M-1&&j==N-1)
      {
        printf("max_chongfu==%d==\n",dp[i][j]);
      }
      if(head->next==NULL)
      {
        V(sem3);
      }else{
        head=head->next;
      }
      }
}

int main(int argc, char *argv[]) {
  // No need to change
  assert(scanf("%s%s", A, B) == 2);
  N = strlen(A);
  M = strlen(B);
  T = !argv[1] ? 1 : atoi(argv[1]);

  
  //初始化

  SEM_INIT(sem2,0);
  SEM_INIT(sem3,0);
  // Add preprocessing code here

  for (int i = 0; i < T; i++) {
    create(Tworker);
  }
  int i=0;
  int j=0;
  for (int round = 0; round < M+N-1; round++) {
  // 1. 计算出本轮能够计算的单元格
  // 2. 将任务分配给线程执行
  // 3. 等待线程执行完毕
  for(int k=0;k<N*M;k++)
        {
          tmp_head->next=(m_list*)malloc(sizeof(m_list));
          tmp_head->next->i=i;
          tmp_head->next->j=j;
          tmp_head->next->next=NULL;
          tmp_head=tmp_head->next;
          V(sem2);
            if((i+j)%2==0)
            {
                if(j==N-1)
                {
                  P(sem3);
                    i++;
                }else if(i==0)
                {
                  P(sem3);
                    j++;
                }else
                {
                    i--;
                    j++;
                }
            }else
            {
                if(i==M-1)
                {
                  P(sem3);
                    j++;
                }else if(j==0)
                {
                  P(sem3);
                    i++;
                }else
                {
                    i++;
                    j--;
                }
            }
        }
}
  join();  // Wait for all workers

  printf("%d\n", result);
}
