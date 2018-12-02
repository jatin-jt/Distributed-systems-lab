#include <bits/stdc++.h>
using namespace std;
vector<int> adj[1000];
int COORDINATOR;
int visited[1000]={0};
int dfs(int u, int p)
{
	if(visited[u]) return -1;
	visited[u] = 1;
	int temp = u;
	for(auto it:adj[u])
	{

		int x = dfs(it,u);
		temp = max(temp,x);
	}
	return temp;
}
int main(int argc, char const *argv[])
{
	int n,m,a,b,initiator;
	cin>>n>>m;
	for (int i = 0; i < m; ++i)
	{
		cin>>a>>b;
		adj[a].push_back(b);
		adj[b].push_back(a);
	}
	cout<<"Initiator node:";
	cin>>initiator;
	COORDINATOR = dfs(initiator,-1);
	cout<<"Coordinator is:"<<COORDINATOR<<endl;
	return 0;
}