using namespace std;

int
main ()
{
  char*   s = "Hallo";
  char**  s_vec[1];

  s_vec[0] = &s;

  return true;
}
