

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>
#include <mutex>
#include "vec2d.h"

using namespace std;
namespace asio = boost::asio;
namespace ip = asio::ip;

class Compress3;
class World;

//通信用クラス
// - UDPとTCPの併用
// ** UDP … データ通信
// ** TCP … 接続担保
class IPCom
{

	asio::io_service io_service_tcp_;
	asio::io_service io_service_udp_;

	u_short recv_tcp_port_ = 58800;
	u_short recv_udp_port_ = 58800;
	u_short send_tcp_port_ = 58900;
	u_short send_udp_port_ = 58900;

	ip::tcp::socket tcp_sock_;
	ip::tcp::acceptor acceptor_;

	ip::udp::socket udp_sock_;
	ip::udp::endpoint udp_ep_;

	//receive buffer
	asio::streambuf tcp_rb_;
	std::array<int8_t, 128> udp_rb_;

	bool stop_com = false;

	void on_accept(const boost::system::error_code& error);
	void on_receive_tcp(const boost::system::error_code& error, std::size_t len);
	//void on_send_tcp(const boost::system::error_code& error, std::size_t len);

	void accept_tcp();
	void receive_tcp();
	//void send_tcp();

	void on_receive_udp(const boost::system::error_code& error, std::size_t len);
	void on_send_udp(const boost::system::error_code& error, std::size_t len);

	void receive_udp_async();
	void receive_udp();
	void send_udp_async();
	void send_udp();

	uint32_t fbo;
	uint32_t data_tex;

	//std::vector<uint8_t> dpxls; //毎フレームバッファ
	//std::vector<uint8_t> spxls; //送信時用フレームバッファ
	std::vector<uint8_t> sdata; //送信符号データ
	std::vector<std::vector<int8_t>> s_buf; //送信時バッファ
	uint8_t finish_buf[2] = {0xff, 0xff}; //送信終了サイン

	int buf_w, buf_h;
	int data_w, data_h;

	int dsize; //送信データサイズ
	const int MAXBS = 1450; //パケットサイズ
	const int TDATA_S = (MAXBS - 2); //実質データサイズ
	const int TEX_MAX; //テクスチャサイズ

	std::future<void> th_tcp;
	std::future<void> th_udp_send;
	std::future<void> th_udp_recv;

	std::unique_ptr<Compress3> compressor;

	//タッチ情報
	std::array<vec2d, 10> touch_pos;
	uint8_t	touch_maxidx = 0;

	std::mutex udp_recv_mtx;

public:

	IPCom(int bw, int bh, int dw, int dh);
	~IPCom();

	//メインスレッド上の更新
	void main_update(World* p_world);

	//void start_sevice();
};