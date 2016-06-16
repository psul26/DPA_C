#include <string>
 
class Trace 
{
	public:
		Trace(const char* bytes);
		Trace(const Trace * t);
		Trace(int length); 
		
		~Trace();
		Trace & operator+=(const Trace& t);
		Trace * getDifferential(const Trace & t) const;
		float getMark() const;
		const char* getValues() const; 
		
	float *v;
	int nTraces;
	int length;	
	
};
