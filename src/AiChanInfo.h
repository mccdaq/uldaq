/*
 * AiChanInfo.h
 *
 *      Author: Measurement Computing Corporation
 */

#ifndef AICHANINFO_H_
#define AICHANINFO_H_

#include "ul_internal.h"
#include <vector>
#include "interfaces/UlAiChanInfo.h"

namespace ul
{

class UL_LOCAL AiChanInfo: public UlAiChanInfo
{
public:
	virtual ~AiChanInfo();
	AiChanInfo(int chan);

	void addChanMode(AiInputMode mode);
	void setChanTypes(long long types);
	int getChanNum() const;
	std::vector<AiInputMode> getChanModes() const;
	AiChanType getChanTypes() const;

private:
	int mChanNum;
	AiChanType mTypes;
	std::vector<AiInputMode> mMode;

	//TcInfo mTcInfo;
};

} /* namespace ul */

#endif /* AICHANINFO_H_ */
