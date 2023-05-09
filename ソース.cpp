#include <iostream>
#include <cstdint>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>

#define Interface struct

//this is network trafic simmulator.
//but low quality.


class StopWatch {
	std::chrono::high_resolution_clock::time_point S;
	std::chrono::high_resolution_clock::time_point E;
public:

	typedef std::chrono::milliseconds TimeType;

	StopWatch() { Start(); };

	bool Start() {
		S = E = std::chrono::high_resolution_clock::now();
		return true;
	}
	bool ReStart() {
		return Start();
	}
	bool Stop() {
		E = std::chrono::high_resolution_clock::now();
		return true;
	}

	bool Reset() {
		S = E = std::chrono::high_resolution_clock::now();
		return true;
	}
	template<class T = TimeType>
	T Ellipse() const {
		return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - S);
	}
	template<class T = TimeType>
	T Result() const {
		return std::chrono::duration_cast<T>(E - S);
	}

};

Interface INetworkObject{
	virtual std::size_t Sender(const std::vector<std::uint8_t>&)=0;
	virtual std::vector<std::uint8_t> Taker(std::size_t N)=0;
	virtual ~INetworkObject() {}
};

class Server :public INetworkObject{
public:

	Server() { Running = true; }

	std::size_t Sender(const std::vector<std::uint8_t>& In) {
		if (SendByte >= Limit) { std::cout << "SendByte is UPUP." << std::endl; return 0; }
		for (auto& o : In) { Buffer.push_back(o); }
		std::size_t L = Write(Buffer, 1024 * 1024);
		SendByte += L;
		return L;
	}

	std::vector<std::uint8_t> Taker(std::size_t N) {
		std::size_t L = std::min({ Limit,Limit - SendByte,N });
		auto V = Read(Buffer, L);
		return V;
	}

	bool ToEnd() { Running = false; return Running; }
	bool ResetSendByte() { SendByte = 0; return true; }

	bool IsRun() { return Running; }
protected:

	std::vector<std::uint8_t> Read(std::vector<std::uint8_t>& In, std::size_t N) {
		std::size_t L = std::min({ N,Limit,In.size() });
		std::vector<std::uint8_t> R(In.begin(), In.begin() + L);

		In.erase(In.begin(), In.begin() + L);
		return R;
	}

	std::size_t  Write(std::vector<std::uint8_t>& In,std::size_t N) {
		std::int64_t L = std::min({ N, Limit,Limit-SendByte ,In.size()});
		Buffer.erase(Buffer.begin(), Buffer.begin() + L);
		return L;
	}
protected:
	std::size_t SendByte = 0;
	std::size_t Limit = -1;
	std::vector<std::uint8_t> Buffer;
	bool Running = false;
	std::mt19937 mt;
};

class Client :public INetworkObject{

public:
	Client(INetworkObject& In):Server(&In){}
	std::size_t Sender(const std::vector<std::uint8_t>& In) {
		return Server->Sender(In);
	}
	std::vector<std::uint8_t> Taker(std::size_t N) {
		return Server->Taker(N);
	
	}
	bool ToEnd() {
		Server = nullptr;
		return true;
	}
protected:
/** /
	std::size_t Write(const std::vector<std::uint8_t>& In, std::size_t N) {
		std::size_t L = std::min({N,In.size(),Limit});
		//Buffer.insert(Buffer.end(), In.begin(), In.begin() + L);
		return Server.Sender()
		
	}
/**/
protected:
	std::vector<std::uint8_t> Buffer;
	std::size_t Limit = 1024;
	INetworkObject* Server=nullptr;
};

int main() {
	Server S;
	Client C(S);
	StopWatch SW;
	StopWatch SW2;

	std::mt19937 mt;

	SW.Start();
	SW2.Start();
	while (S.IsRun()) {
		if (SW.Ellipse<std::chrono::milliseconds>().count() >= 1000) {

			S.ResetSendByte();
			SW.Reset();
		}
		if (SW2.Ellipse<std::chrono::milliseconds>().count() >= 10000) {
			C.ToEnd();
			S.ToEnd();
			break;
		}

		auto A = C.Taker(1024);
		std::vector<std::uint8_t> B;

		for (std::size_t i = 0; i < 2048; i++) {
			B.push_back(mt());
		}

		std::size_t L = C.Sender(B);
	}

	return 0;
}