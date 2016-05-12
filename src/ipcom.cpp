
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include <future>
#include "ipcom.h"
#include "compressor.h"
#include "world.h"



void IPCom::on_accept(const boost::system::error_code & error)
{
	if(error)
	{
		std::cout << "accept failed: " << error.message() << std::endl;
	}
	else
	{
		std::cout << "accept correct!" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		//TCP受信待機
		receive_tcp();
		//接続先の情報を得る
		auto tcp_ep = tcp_sock_.remote_endpoint();
		udp_ep_ = ip::udp::endpoint(tcp_ep.address(), send_udp_port_);
		//UDP送信
		th_udp_send = std::async(std::launch::async, [&] {
			send_udp();
		});
		//UDP受信
		th_udp_recv = std::async(std::launch::async, [&] {
			receive_udp();
		});
		//send_udp_async();
		//receive_udp();
		//UDPサービス開始
		//th_udp_send = std::async(std::launch::async, [&] {
		//	io_service_udp_.run();
		//});
	}
}

void IPCom::on_receive_tcp(const boost::system::error_code & error, std::size_t len)
{
	try
	{
		std::istream is(&tcp_rb_);
		std::string data;
		is >> data;

		if(error && error != asio::error::eof)
		{
			cout << "受信失敗 : " << error.message() << endl;
			throw "exit";
		}
		else if(data == "end")
		{
			cout << "接続切断" << endl;
			throw "exit";
		}
		else
		{
			receive_tcp();
		}
	}
	catch(...)
	{
		cout << "通信停止" << endl;
		//現在の通信を停止（UDP）
		stop_com = true;
		//再接続
		accept_tcp();
	}
}

//void IPCom::on_send_tcp(const boost::system::error_code & error, std::size_t len)
//{
//}

void IPCom::receive_tcp()
{
	boost::asio::async_read(tcp_sock_, tcp_rb_, asio::transfer_at_least(1),
		boost::bind(&IPCom::on_receive_tcp, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void IPCom::on_receive_udp(const boost::system::error_code & error, std::size_t len)
{
	/*
	try
	{
		const std::string data(udp_rb_.data(), len);

		if(error && error != asio::error::eof)
		{
			std::cout << "receive failed: " << error.message() << std::endl;
			throw "exit";
		}
		else if(data == "end")
		{
			cout << "クライアントが接続を切りました\n";
			throw "exit";
		}
		else
		{
			// ...
			cout << data << endl;
		}
	}
	catch(...)
	{
		cout << "送信停止" << endl;
		stop_com = true;
	}
	*/
}

void IPCom::on_send_udp(const boost::system::error_code & error, std::size_t len)
{
	if(error)
	{
		std::cout << "send failed: " << error.message() << std::endl;
		stop_com = true;
	}
	else
	{
		std::cout << "send correct!" << std::endl;
	}
}

void IPCom::receive_udp_async()
{
	if(!stop_com)
	{
		udp_sock_.async_receive_from(boost::asio::buffer(udp_rb_), udp_ep_,
			boost::bind(&IPCom::on_receive_udp, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
	}
}

void IPCom::receive_udp()
{
	while(!stop_com)
	{
		try
		{
			boost::system::error_code error;
			auto len = udp_sock_.receive_from(boost::asio::buffer(udp_rb_), udp_ep_, 0, error);
			{
				std::lock_guard<std::mutex> lock(udp_recv_mtx);
				touch_maxidx = (uint8_t)udp_rb_[0];
				float* fv = reinterpret_cast<float*>(&udp_rb_[1]);
				//cout << (int)touch_maxidx << " :: ";
				for(int i = 0; i < touch_maxidx; ++i)
				{
					touch_pos[i].x = fv[2 * i + 0];
					touch_pos[i].y = fv[2 * i + 1];
					//cout << "(" << touch_pos[i].x << ", " << touch_pos[i].y << ") ";
				}
				//cout << endl;
			}
		}
		catch(std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			break;
		}
	}
}

void IPCom::send_udp_async()
{
	if(!stop_com)
	{
		/*
		{
			std::lock_guard<std::mutex> locks(com_mtx);
			//for(int i = 0; i < data_w * data_h * 3; ++i)
			//{
			//	spxls[i] = dpxls[i];
			//}
			compressor->compress();
		}
		*/
		//符号データ取得
		{
			std::lock_guard<std::mutex> locks(compressor->get_mtx());
			dsize = compressor->get_codes_size();
			for(int i = 0; i < dsize; ++i)
			{
				sdata[i] = compressor->get_codes()[i];
			}
		}


		assert(dsize / TDATA_S < 65536);

		for(int send_index = 0; send_index <= dsize / TDATA_S; ++send_index)
		{
			s_buf[send_index][0] = (send_index >> 8) & 0xff;
			s_buf[send_index][1] = send_index & 0xff;
			size_t cpy_size;
			int mem_addr = send_index * TDATA_S;
			if(mem_addr + TDATA_S >= dsize)
			{
				cpy_size = TDATA_S - (mem_addr + TDATA_S - dsize);
			}
			else
			{
				cpy_size = TDATA_S;
			}
			memcpy(&s_buf[send_index][2], &sdata[mem_addr], cpy_size);

			udp_sock_.async_send_to(boost::asio::buffer(s_buf[send_index]), udp_ep_,
				boost::bind(&IPCom::on_send_udp, this, asio::placeholders::error, asio::placeholders::bytes_transferred));

			//udp_sock_.async_send_to(boost::asio::buffer(s_buf[send_index]), asio::ip::udp::endpoint(asio::ip::address::from_string("127.0.0.1"), udp_port_),
			//	boost::bind(&IPCom::on_send_udp, this, asio::placeholders::error, asio::placeholders::bytes_transferred));

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		cout << "finish sening tex" << endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		send_udp_async();
	}
}

void IPCom::send_udp()
{
	while(!stop_com)
	{
		/*
		{
		std::lock_guard<std::mutex> locks(com_mtx);
		//for(int i = 0; i < data_w * data_h * 3; ++i)
		//{
		//	spxls[i] = dpxls[i];
		//}
		compressor->compress();
		}
		*/
		//符号データ取得
		{
			std::lock_guard<std::mutex> locks(compressor->get_mtx());
			dsize = compressor->get_codes_size();
			for(int i = 0; i < dsize; ++i)
			{
				sdata[i] = compressor->get_codes()[i];
			}
		}


		assert(dsize / TDATA_S < 65536);

		for(int send_index = 0; send_index <= dsize / TDATA_S; ++send_index)
		{
			s_buf[send_index][0] = (send_index >> 8) & 0xff;
			s_buf[send_index][1] = send_index & 0xff;
			size_t cpy_size;
			int mem_addr = send_index * TDATA_S;
			if(mem_addr + TDATA_S >= dsize)
			{
				cpy_size = TDATA_S - (mem_addr + TDATA_S - dsize);
			}
			else
			{
				cpy_size = TDATA_S;
			}
			memcpy(&s_buf[send_index][2], &sdata[mem_addr], cpy_size);

			udp_sock_.send_to(boost::asio::buffer(s_buf[send_index]), udp_ep_);

			//std::this_thread::sleep_for(std::chrono::milliseconds(8));
		}
		//終了サイン送信
		udp_sock_.send_to(boost::asio::buffer(finish_buf), udp_ep_);

		cout << "finish sening tex" << endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(60));
	}
}

//void IPCom::send_tcp()
//{
//}



void IPCom::accept_tcp()
{
	acceptor_.async_accept(
		tcp_sock_,
		boost::bind(&IPCom::on_accept, this, asio::placeholders::error));
}


IPCom::IPCom(int bw, int bh, int dw, int dh)
  :	tcp_sock_(io_service_tcp_),
	udp_sock_(io_service_udp_, ip::udp::endpoint(ip::udp::v4(), recv_udp_port_)),
	acceptor_(io_service_tcp_, ip::tcp::endpoint(ip::tcp::v4(), recv_tcp_port_)),
	buf_w(bw), buf_h(bh), data_w(dw), data_h(dh),
	TEX_MAX(dw * dh * 3)
{
	//FBOの作成
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glGenTextures(1, &data_tex);
	glBindTexture(GL_TEXTURE_2D, data_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data_w, data_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_tex, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//バッファーデータ
	//dpxls.resize(data_w * data_h * 3);
	//spxls.resize(data_w * data_h * 3);
	sdata.resize(data_w * data_h * 3);
	s_buf.resize(data_w * data_h * 3 / TDATA_S + 1);
	for(auto& s_block : s_buf)
	{
		s_block.resize(MAXBS);
	}
	//圧縮機
	compressor = std::make_unique<Compress3>(data_w, data_h, fbo, data_tex);

	accept_tcp();
	//TCPサービス開始
	th_tcp = std::async(std::launch::async, [&] {
		io_service_tcp_.run();
	});
}

IPCom::~IPCom()
{
	if(th_tcp.valid())
		th_tcp.get();
	if(th_udp_send.valid())
		th_udp_send.get();
}

void IPCom::main_update(World* p_world)
{
	//カレントバッファのコピー
	{
		//std::lock_guard<std::mutex> lock(com_mtx);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		constexpr GLenum buf_fbcpy[1] = {GL_COLOR_ATTACHMENT0_EXT};
		glDrawBuffers(1, buf_fbcpy);
		glBlitFramebuffer(0, 0, buf_w, buf_h, 0, 0, data_w, data_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	compressor->compress();

	//タッチ情報の更新
	{
		std::lock_guard<std::mutex> lock(udp_recv_mtx);
		p_world->set_touch_data(touch_pos, touch_maxidx);
	}

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, buf_w, buf_h);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, data_tex);
	glBegin(GL_QUADS);
	glColor3d(1.0, 1.0, 1.0);
	glTexCoord2d(0.0, 0.0);
	glVertex2d(-1.0, 0.5);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(-0.5, 0.5);
	glTexCoord2d(1.0, 1.0);
	glVertex2d(-0.5, 1.0);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(-1.0, 1.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	*/
	
}
