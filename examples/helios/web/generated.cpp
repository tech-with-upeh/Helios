#include <iostream>
#include "vdom.hpp"
#include <format>
#include <cmath>
using namespace std;

void updateUI() {
        // Re-render the current page
        if (GlobalState::getCurrentPage()) {
            GlobalState::getCurrentPage()->render(true);
        }
    }
auto page_1 = make_shared<VPage>();
int main() {
	Router::add("/",page_1);
	std::string homepage = R"(
		.mydiv{ 
			height : 50px;;
			width : 100%;;
			position : fixed;;
			top : 0px;;
			display : flex;;
			align-items : center;;
			justify-content : space-between;;
			border-bottom-right-radius : 20px;;
			border-bottom-left-radius : 20px;;
			z-index : 999;
		}	.brandlogo{ 
			height : 30px;;
			width : 30px;;
			object-fit : contain;;
		}	.mydivtext{ 
			font-weight : bold;;
			font-size : 20px;;
			text-align : center;;
			user-select : none;;
			cursor : pointer;;
		}	.bgimg{ 
			position : absolute;
			top : 0;
			left : 0px;
			width : 100%;
			height : 100%;
			object-fit : cover;
			z-index : 1;
			opacity : 0.2;
		})";
	
	page_1->builder = [&, homepage](VPage& page, std::string msg) {
		page.setTitle("Helios ~ Your Full Stack FrmeWork");
		page.bodyAttrs["style"] = "background-color:black;color:white;padding:0px;margin:0px;";
		page.addScript("anim.js");
		page.addStylesheet("./global.css");
		page.addStylesheet("https://cdn.lineicons.com/5.1/line/lineicons.css");
		page.setFavicon("logo.png");

		page.addStyle("homepage", homepage);

		auto h = Platform().height();
		auto w = Platform().width();
		
	auto num = make_shared<appstate::State<int>>("num",0);
		
	auto navh = make_shared<appstate::State<std::string>>("navh","0px");
		
	auto navpad = make_shared<appstate::State<std::string>>("navpad","0px");
		
	VNode canvas_2("canvas", "", "__ink_243");
		canvas_2.type = VNodeType::CANVAS;
		canvas_2.setAttr("id", "bg");
		canvas_2.height = h;
		canvas_2.width = w;

	page.addChild(canvas_2);
		
	VNode img_3("img", "", "__ink_244");
	img_3.setAttr("src", "box-bg.jpg");
	img_3.setAttr("class",  "bgimg");

	page.addChild(img_3);
		
	VNode view_4("div", "", "__ink_245");
		view_4.setAttr("id", "mydiv");
	view_4.setAttr("class",  "mydiv");
	view_4.onClick([num]() {
			num->set((num->get() + 1));
		cout << num->get() << endl;
		updateUI();	});

	VNode view_5("div", "", "__ink_246");
		view_5.setAttr("id", "nav-start");
	view_5.setAttr("class",  "navstart");

	VNode img_6("img", "", "__ink_247");
	img_6.setAttr("src", "logo-wh.png");
	img_6.setAttr("class",  "brandlogo");

	view_5.addChild(img_6);
		VNode text_8("p","Helios", "__ink_248");
	text_8.setAttr("class",  "brandname");

	view_5.addChild(text_8);
	view_4.addChild(view_5);
	VNode view_11("div", "", "__ink_249");
		view_11.setAttr("id", "nav-end");
	view_11.setAttr("class",  "navend");

		VNode text_12("p",to_string(num->get()), "__ink_250");
	text_12.setAttr("class",  "mydivtext");

	view_11.addChild(text_12);
	VNode img_14("img", "", "__ink_251");
	img_14.setAttr("src", "nav.png");
	img_14.setAttr("class",  "navicon");
	img_14.onClick([navh, navpad]() {
			if(navh->get() == string("0px")){
    	navh->set(string("150px"));
    	navpad->set(string("20px"));
    
    }
    else {	navh->set(string("0px"));	navpad->set(string("0px"));
    }
		updateUI();	});

	view_11.addChild(img_14);
	VNode view_16("div", "", "__ink_252");
		view_16.setAttr("id", "nav-menu");
	view_16.setAttr("class",  "navmenu");
	view_16.setAttr("style", "padding:"+navpad->get()+";"+"height:"+navh->get()+";"+"");

		VNode text_17("p",navh->get(), "__ink_253");

	view_16.addChild(text_17);
		VNode text_19("p","Docs", "__ink_254");

	view_16.addChild(text_19);
		VNode text_21("p","GitHub", "__ink_255");

	view_16.addChild(text_21);
	view_11.addChild(view_16);
	view_4.addChild(view_11);
	page.addChild(view_4);
		
	VNode view_25("div", "", "__ink_256");
		view_25.setAttr("id", "hero");
	view_25.setAttr("class",  "hero");

	VNode view_26("div", "", "__ink_257");
		view_26.setAttr("id", "hero-start");
	view_26.setAttr("class",  "herostart");

		VNode text_27("p","Build Once, Render Everywhere", "__ink_258");
	text_27.setAttr("class",  "herotitle");

	view_26.addChild(text_27);
		VNode text_29("p","Your Full Stack Framework", "__ink_259");
	text_29.setAttr("style", "font-size:20px;;color:#888;;");

	view_26.addChild(text_29);
	VNode view_31("div", "", "__ink_260");
		view_31.setAttr("id", "herobtn");

		VNode text_32("p","Download Helios", "__ink_261");

	view_31.addChild(text_32);
	view_26.addChild(view_31);
	VNode view_35("div", "", "__ink_262");
		view_35.setAttr("id", "seedocs");

		VNode text_36("p","see docs", "__ink_263");

	view_35.addChild(text_36);
	view_26.addChild(view_35);
	view_25.addChild(view_26);
	VNode view_40("div", "", "__ink_264");
		view_40.setAttr("id", "hero-terminal");
	view_40.setAttr("style", "background-color:#222;;padding:20px;;border-radius:5px;;font-family:monospace;;font-size:14px;;");

	VNode view_41("div", "", "__ink_265");
		view_41.setAttr("id", "terminal-header");
	view_41.setAttr("style", "display:flex;;align-items:center;;margin-bottom:10px;;");

	VNode view_42("div", "", "__ink_266");
		view_42.setAttr("id", "terminal-header-start");
	view_42.setAttr("class",  "terminalheaderstart");
	view_42.setAttr("style", "display:flex;;align-items:center;;border-radius:50%;;margin-right:5px;;");

	VNode view_43("div", "", "__ink_267");
		view_43.setAttr("id", "terminal-dot");
	view_43.setAttr("style", "height:10px;;width:10px;;background-color:#f00;;border-radius:50%;;margin-right:5px;;");

	view_42.addChild(view_43);
	VNode view_45("div", "", "__ink_268");
		view_45.setAttr("id", "terminal-dot");
	view_45.setAttr("style", "height:10px;;width:10px;;background-color:#0f0;;border-radius:50%;;margin-right:5px;;");

	view_42.addChild(view_45);
	VNode view_47("div", "", "__ink_269");
		view_47.setAttr("id", "terminal-dot");
	view_47.setAttr("style", "height:10px;;width:10px;;background-color:#ff0;;border-radius:50%;;margin-right:5px;;");

	view_42.addChild(view_47);
	view_41.addChild(view_42);
		VNode text_50("p","Index.ink", "__ink_270");
	text_50.setAttr("style", "color:#888;;");

	view_41.addChild(text_50);
	view_40.addChild(view_41);
	VNode view_53("div", "", "__ink_271");
		view_53.setAttr("id", "hr");
	view_53.setAttr("style", "height:1px;;background-color:#333;;margin:10px 0;;");

	view_40.addChild(view_53);
	view_25.addChild(view_40);
	page.addChild(view_25);
		};
	EM_ASM({
		Module._handleRoute(allocateUTF8(window.location.pathname));
		window.addEventListener("popstate", () => {
		Module._handleRoute(allocateUTF8(window.location.pathname));
		});
	});return 0;
}
