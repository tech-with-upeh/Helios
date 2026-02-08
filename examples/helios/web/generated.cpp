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
	
	page_1->builder = [&, homepage](VPage& page) {
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
		
	VNode canvas_2("canvas", "", "__ink_27");
		canvas_2.type = VNodeType::CANVAS;
		canvas_2.setAttr("id", "bg");
		canvas_2.height = h;
		canvas_2.width = w;

	page.addChild(canvas_2);
		
	VNode img_3("img", "", "__ink_28");
	img_3.setAttr("src", "box-bg.jpg");
	img_3.setAttr("class",  "bgimg");

	page.addChild(img_3);
		
	VNode view_4("div", "", "__ink_29");
		view_4.setAttr("id", "mydiv");
	view_4.setAttr("class",  "mydiv");
	view_4.onClick([num]() {
			num->set((num->get() + 1));
		cout << num->get() << endl;
		updateUI();	});

	VNode view_5("div", "", "__ink_30");
		view_5.setAttr("id", "nav-start");
	view_5.setAttr("class",  "navstart");

	VNode img_6("img", "", "__ink_31");
	img_6.setAttr("src", "logo-wh.png");
	img_6.setAttr("class",  "brandlogo");

	view_5.addChild(img_6);
		VNode text_8("p","Helios", "__ink_32");
	text_8.setAttr("class",  "brandname");

	view_5.addChild(text_8);
	view_4.addChild(view_5);
	VNode view_11("div", "", "__ink_33");
		view_11.setAttr("id", "nav-end");
	view_11.setAttr("class",  "navend");

		VNode text_12("p",to_string(num->get()), "__ink_34");
	text_12.setAttr("class",  "mydivtext");

	view_11.addChild(text_12);
	VNode img_14("img", "", "__ink_35");
	img_14.setAttr("src", "nav.png");
	img_14.setAttr("class",  "navicon");

	view_11.addChild(img_14);
	view_4.addChild(view_11);
	page.addChild(view_4);
		};
	EM_ASM({
		Module._handleRoute(allocateUTF8(window.location.pathname));
		window.addEventListener("popstate", () => {
		Module._handleRoute(allocateUTF8(window.location.pathname));
		});
	});return 0;
}
