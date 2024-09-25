#include "ServerEngine.hpp"
#include <cstddef>

class AClientFdReader
{
  public:
	bool readClientFd(
		ServerEngine &ServerEngine,
		size_t		  pollFDIndex,
		size_t		  clientIndex
	);

  private:
	AClientFdReader(void);
	AClientFdReader(const AClientFdReader &src);
	~AClientFdReader(void);
	AClientFdReader &operator=(const AClientFdReader &rhs);

	// extract value of header

	// extract size if chunked

};

