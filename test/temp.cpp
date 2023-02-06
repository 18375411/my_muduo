 #include<string>
 #include<map>
 #include<iostream>
 using namespace std;
class Solution {
public:
    int lengthOfLongestSubstring(string s) {
        int left=0,right=-1;
        int maxLen=0;
        map<char,int> temp;

        for(left=0;left<s.length();left++)
        {
            if(left!=0)
            {
                temp[s[left-1]]--;
            }
            while(right+1<s.length()  &&  temp.count(s[right+1])==0)
            {
                temp[s[right+1]]++;
                right++;
            }
            std::cout<<"left:"<<left<<"right:"<<right<<endl;
            maxLen=max(maxLen,right-left+1);
        }
       
        return maxLen;
    }
};
int main()
{
    string s="pwwkew";
    Solution A;
    cout<<A.lengthOfLongestSubstring(s);

}