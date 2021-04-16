#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <random>
#include <stack>
#include <chrono>

using namespace std;

typedef long long ll;

int N;
ll A_best;
vector<vector<ll>> weights;
ofstream fout;

vector<int> permutation;
random_device rd;
mt19937 g(rd());

ll get_random_tour(vector<int>& edges)
{
	shuffle(permutation.begin(), permutation.end(), g);
	edges = vector<int>(N, 0);
	ll ans = 0;
	for (int i = 0; i < N - 1; i++)
	{
		edges[permutation[i]] = permutation[i + 1];
		ans += weights[permutation[i]][permutation[i + 1]];
	}
	edges[permutation[N - 1]] = permutation[0];
	ans += weights[permutation[N - 1]][permutation[0]];
	return ans;
}

ll _2opt(vector<int>& edges, ll len)
{
	ll best = len;
	int tmp_a, tmp_b, tmp_c, tmp_d;

	for (int i = 0; i < N; i++)
	{
		for (int j = i + 1; j < N; j++)
		{
			// cannot use same nor consecutive edges
			if (i != j && edges[i] != j && edges[j] != i)
			{
				// ...~~~> a->b ~~~~~~> c->d ~~~...
				int a = i, b = edges[i];
				int c = j, d = edges[j];

				// acbd
				ll tmp = len - weights[a][b] - weights[c][d] + weights[a][c] + weights[b][d];
				if (tmp < best)
				{
					best = tmp;
					tmp_a = a;
					tmp_b = b;
					tmp_c = c;
					tmp_d = d;
					//goto l;

					int prev = tmp_b, cur = edges[tmp_b], next = edges[edges[tmp_b]];
					do
					{
						edges[cur] = prev;
						prev = cur;
						cur = next;
						next = edges[next];
					} while (prev != tmp_c);

					edges[tmp_a] = tmp_c;
					edges[tmp_b] = tmp_d;
					//return best;
				}
			}
		}
	}
l:
	return best;
}

ll __2opt(vector<int>& edges, ll len)
{
	ll ans = len;
	do
	{
		len = ans;
		ans = _2opt(edges, len);
	} while (ans < len);
	return ans;
}


void path_to_edges(vector<int>& path, vector<int>& edges)
{
	//edges.clear();
	for (int i = 0; i < N - 1; i++)
	{
		edges[path[i]] = path[i + 1];
	}
	edges[path[N - 1]] = path[0];
}


ll intersect(vector<int>& edges_1, vector<int>& edges_2, vector<int>& path)
{
	path.clear();
	vector<unordered_set<int>> common(N), diff(N);
	for (int i = 0; i < N; i++)
	{
		diff[i].insert(edges_1[i]);
		diff[edges_1[i]].insert(i);
	}
	for (int i = 0; i < N; i++)
	{
		if (diff[i].find(edges_2[i]) != diff[i].end())
		{
			common[i].insert(edges_2[i]);
			common[edges_2[i]].insert(i);
			diff[i].erase(edges_2[i]);
			diff[edges_2[i]].erase(i);
		}
		else
		{
			diff[i].insert(edges_2[i] + 1000);
			diff[edges_2[i]].insert(i + 1000);
		}
	}

	int comps = 0;
	// search best edges in each component
	vector<bool> used(N, false);
	vector<int> comp;
	for (int i = 0; i < N; i++)
	{
		if (!used[i])
		{
			comp.clear();

			// DFS
			stack<int> st;
			st.push(i);
			while (!st.empty())
			{
				int v = st.top();
				st.pop();
				if (!used[v])
				{
					used[v] = true;
					comp.push_back(v);
					for (auto uu : diff[v])
					{
						auto u = uu < 1000 ? uu : uu - 1000;
						if (!used[u])
						{
							st.push(u);
						}
					}
				}
			}


			if (comp.size() > 1) comps++;
			vector<pair<int, int>> e1, e2;
			ll l1 = 0, l2 = 0;

			for (auto from : comp)
			{
				for (auto to : diff[from])
				{
					if (to < 1000)
					{
						e1.push_back(pair<int, int>(from, to));
						l1 += weights[from][to];
					}
					else
					{
						e2.push_back(pair<int, int>(from, to - 1000));
						l2 += weights[from][to - 1000];
					}
				}
			}
			for (auto pair : (l1 < l2 ? e1 : e2))
			{
				common[pair.first].insert(pair.second);
			}
		}
	}

	// check whether edges form a path
	used = vector<bool>(N, false);
	stack<int> st;
	st.push(0);
	while (!st.empty())
	{
		int v = st.top();
		st.pop();
		if (!used[v])
		{
			used[v] = true;
			path.push_back(v);
			for (auto u : common[v])
			{
				if (!used[u])
				{
					st.push(u);
				}
			}
		}
	}

	//if (comps > 1) 
	cout << comps << endl;
	if (path.size() != N)
	{
		return -1;
	}
	else
	{
		ll ans = 0;
		for (int i = 0; i < N - 1; i++)
		{
			ans += weights[path[i]][path[i + 1]];
		}
		ans += weights[path[N - 1]][path[0]];
		return ans;
	}
}


void check_and_exit(ll len, vector<int>& edges)
{
	if (len < A_best * 1.05)
	{
		int v = 0;
		for (int i = 0; i < N; i++)
		{
			fout << v + 1 << " ";
			v = edges[v];
		}
		exit(0);
	}
}

void _exit(ll len, vector<int>& edges)
{
	cerr << len;
	int v = 0;
	for (int i = 0; i < N; i++)
	{
		fout << v + 1 << " ";
		v = edges[v];
	}
	exit(0);
}

int main()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	ios_base::sync_with_stdio(0);
	cin.tie(0);
	cout.tie(0);
	ifstream fin("tsp.in");
	fout = ofstream("tsp.out");
	fin.tie(0);
	fout.tie(0);

	fin >> N;
	for (int i = 0; i < N; i++)
	{
		weights.push_back(vector<ll>(N, 0));
		permutation.push_back(i);
	}
	for (int i = 1; i < N; i++)
	{
		for (int j = 0; j < i; j++)
		{
			fin >> weights[i][j];
			weights[j][i] = weights[i][j];
		}
	}


	vector<int> edges_1;
	ll len_1;
	len_1 = get_random_tour(edges_1);
	len_1 = __2opt(edges_1, len_1);
	vector<int> ans;

	check_and_exit(len_1, edges_1);

	while (std::chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - begin).count() < 10000) //(true) //
	{
		vector<int> edges_2;
		ll len_2;
		len_2 = get_random_tour(edges_2);
		len_2 = __2opt(edges_2, len_2);

		check_and_exit(len_2, edges_2);

		ans.clear();
		ll tmp = intersect(edges_1, edges_2, ans);
		cout << len_1 << " " << len_2 << " " << tmp << endl;
		if (tmp == -1)
		{
			continue;
		}
		else
		{
			len_1 = tmp;
		}

		if (len_1 < A_best * 1.05)
		{
			for (auto i : ans)
			{
				fout << i + 1 << " ";
			}
			return 0;
		}


		path_to_edges(ans, edges_1);
		//len_1 = __2opt(edges_1, len_1);
		check_and_exit(len_1, edges_1);

	}
	path_to_edges(ans, edges_1);
	_exit(len_1, edges_1);
}