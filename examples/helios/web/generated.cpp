#include <iostream>
#include "vdom.hpp"
#include <format>
#include <cmath>
#include <vector>
#include <any>
using namespace std;

void updateUI() {
        // Re-render the current page
        if (GlobalState::getCurrentPage()) {
            GlobalState::getCurrentPage()->render(true);
        }
    }
auto page_1 = make_shared<VPage>();
int main() {
	EM_ASM({
		Module.domCache = {};
	});
	
	Router::add("/",page_1);
	std::string homepage = R"(
	.mydiv {
			height : 70px;;
			width : 100%;;
			position : fixed;;
			top : 0px;;
			display : flex;;
			align-items : center;;
			justify-content : space-between;;
			z-index : 999;
			transition : background-color 0.3s ease;;
		}.brandlogo {
			height : 30px;;
			width : 30px;;
			object-fit : contain;;
		}.mydivtext {
			font-weight : bold;;
			font-size : 20px;;
			text-align : center;;
			user-select : none;;
			cursor : pointer;;
		}.bgimg {
			position : absolute;
			top : 0;
			left : 0px;
			width : 100%;
			height : 100%;
			object-fit : cover;
			z-index : 1;
			background-color : #191a1f8c;
			background-image : linear-gradient(to right, rgba(140, 43, 238, 0.1) 1px, transparent 1px), linear-gradient(to bottom, rgba(140, 43, 238, 0.1) 1px, transparent 1px);
			background-size : 40px 40px;
		}
		@media only screen and (min-width: 992px) {
			#hero {
			flex-direction : row;;
		}
			#hero-terminal {
			width : 40% !important;;
			height : fit-content !important;;
		}
			.herotitle, .herotitlespan {
			font-size : 4em;
		}
			.features #feature-list {
			flex-direction : row !important;;
		}
			.features #feature-list #feature-grp {
			flex-direction : row !important;;
		}
		}
)";
	
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
		
	auto scrollYState = make_shared<appstate::State<double>>("scrollYState",0.0);
		
	auto navBgColor = make_shared<appstate::State<std::string>>("navBgColor","transparent");
		page.addevent("scroll", [&, num, navh, navpad, scrollYState, navBgColor]() {
				scrollYState->set(Platform().scrollY());
				if(scrollYState->get() > 50.0){
    	navBgColor->set(string("rgb(114, 52, 230);"));
    
    }
    else {	navBgColor->set(string("transparent"));
    }
updateUI();
		});

		page.addevent("resize", [&, num, navh, navpad, scrollYState, navBgColor]() {
updateUI();
		});

		
	VNode canvas_2("canvas", "", "__ink_0");
		canvas_2.type = VNodeType::CANVAS;
		canvas_2.setAttr("id", "bg");
		canvas_2.height = h;
		canvas_2.width = w;

	page.addChild(canvas_2);
		
	VNode view_3("div", "", "__ink_1");
		view_3.setAttr("id", "herobg");
	view_3.setAttr("class",  "bgimg");

	page.addChild(view_3);
		
	VNode view_4("div", "", "__ink_2");
		view_4.setAttr("id", "mydiv");
	view_4.setAttr("class",  "mydiv");
	view_4.onClick([num]() {
			num->set((num->get() + 1));
		cout << num->get() << endl;
		updateUI();	});
	view_4.setAttr("style", "background-color:"+navBgColor->get()+";"+"");

	VNode view_5("div", "", "__ink_3");
		view_5.setAttr("id", "nav-start");
	view_5.setAttr("class",  "navstart");

	VNode img_6("img", "", "__ink_4");
	img_6.setAttr("src", "logo-wh.png");
	img_6.setAttr("class",  "brandlogo");

	view_5.addChild(img_6);
		VNode text_8("p","Helios", "__ink_5");
	text_8.setAttr("class",  "brandname");

	view_5.addChild(text_8);
	view_4.addChild(view_5);	if(w > 992.0){
    
	VNode view_11("div", "", "__ink_6");
		view_11.setAttr("id", "nav-end");
	view_11.setAttr("class",  "navend");

	VNode view_12("div", "", "__ink_7");
		view_12.setAttr("id", "navdesk-med");

		VNode text_13("p","Docs", "__ink_8");

	view_12.addChild(text_13);
		VNode text_15("p","GitHub", "__ink_9");

	view_12.addChild(text_15);
	VNode input_17("input", "", "__ink_10");
		input_17.setAttr("placeholder", "Search docs...");
	input_17.setAttr("class",  "searchinput");

	view_12.addChild(input_17);
	view_11.addChild(view_12);
	VNode view_20("div", "", "__ink_11");
		view_20.setAttr("id", "mavdesk-end");

	VNode view_21("div", "", "__ink_12");
		view_21.setAttr("id", "navdesk-btn");

		VNode text_22("p","Download Helios", "__ink_13");

	view_21.addChild(text_22);
	view_20.addChild(view_21);
	view_11.addChild(view_20);
	view_4.addChild(view_11);
    
    }
    else {
	VNode view_26("div", "", "__ink_14");
		view_26.setAttr("id", "nav-end");
	view_26.setAttr("class",  "navend mobilenav");

		VNode text_27("p",to_string(num->get()), "__ink_15");
	text_27.setAttr("class",  "mydivtext");

	view_26.addChild(text_27);
	VNode img_29("img", "", "__ink_16");
	img_29.setAttr("src", "nav.png");
	img_29.setAttr("class",  "navicon");
	img_29.onClick([navh, navpad]() {
			cout << string("clicked") << endl;
			if(navh->get() == string("0px")){
    	navh->set(string("150px"));
    	navpad->set(string("20px"));
    
    }
    else {	navh->set(string("0px"));	navpad->set(string("0px"));
    }
		updateUI();	});

	view_26.addChild(img_29);
	VNode view_31("div", "", "__ink_17");
		view_31.setAttr("id", "nav-menu");
	view_31.setAttr("class",  "navmenu");
	view_31.setAttr("style", "padding:"+navpad->get()+";"+"height:"+navh->get()+";"+"");

		VNode text_32("p",navh->get(), "__ink_18");

	view_31.addChild(text_32);
		VNode text_34("p","Docs", "__ink_19");

	view_31.addChild(text_34);
		VNode text_36("p","GitHub", "__ink_20");

	view_31.addChild(text_36);
	view_26.addChild(view_31);
	view_4.addChild(view_26);
    }
	page.addChild(view_4);
		
	VNode view_40("div", "", "__ink_21");
		view_40.setAttr("id", "hero");
	view_40.setAttr("class",  "hero");

	VNode view_41("div", "", "__ink_22");
		view_41.setAttr("id", "hero-start");
	view_41.setAttr("class",  "herostart");

	VNode view_42("div", "", "__ink_23");
		view_42.setAttr("id", "herotxt");
	view_42.setAttr("class",  "herotxt");

		VNode text_43("p","Kelz, Render ", "__ink_24");
	text_43.setAttr("class",  "herotitle");

	view_42.addChild(text_43);
		VNode text_45("p","Everywhere", "__ink_25");
	text_45.setAttr("class",  "herotitlespan");

	view_42.addChild(text_45);
	view_41.addChild(view_42);
		VNode text_48("p","Your Full Stack Framework", "__ink_26");
	text_48.setAttr("style", "font-size:20px;;color:#888;;");

	view_41.addChild(text_48);
	VNode view_50("div", "", "__ink_27");
		view_50.setAttr("id", "herobtn");

		VNode text_51("p","Download Helios", "__ink_28");

	view_50.addChild(text_51);
	view_41.addChild(view_50);
	VNode view_54("div", "", "__ink_29");
		view_54.setAttr("id", "seedocs");

		VNode text_55("p","see docs", "__ink_30");

	view_54.addChild(text_55);
	view_41.addChild(view_54);
	view_40.addChild(view_41);
	VNode view_59("div", "", "__ink_31");
		view_59.setAttr("id", "hero-terminal");
	view_59.setAttr("style", "background-color:#222;;padding:20px;;border-radius:15px;;font-family:monospace;;font-size:14px;;");

	VNode view_60("div", "", "__ink_32");
		view_60.setAttr("id", "terminal-header");
	view_60.setAttr("style", "display:flex;;align-items:center;;margin-bottom:10px;;height:6%;;width:100%;");

	VNode view_61("div", "", "__ink_33");
		view_61.setAttr("id", "terminal-header-start");
	view_61.setAttr("class",  "terminalheaderstart");
	view_61.setAttr("style", "display:flex;;align-items:center;;border-radius:50%;;margin-right:5px;;");

	VNode view_62("div", "", "__ink_34");
		view_62.setAttr("id", "terminal-dot");
	view_62.setAttr("style", "height:0.75rem;;width:0.75rem;;background-color:#f00;;border-radius:50%;;margin-right:5px;;");

	view_61.addChild(view_62);
	VNode view_64("div", "", "__ink_35");
		view_64.setAttr("id", "terminal-dot");
	view_64.setAttr("style", "height:0.75rem;;width:0.75rem;;background-color:#0f0;;border-radius:50%;;margin-right:5px;;");

	view_61.addChild(view_64);
	VNode view_66("div", "", "__ink_36");
		view_66.setAttr("id", "terminal-dot");
	view_66.setAttr("style", "height:0.75rem;;width:0.75rem;;background-color:#ff0;;border-radius:50%;;margin-right:5px;;");

	view_61.addChild(view_66);
	view_60.addChild(view_61);
		VNode text_69("p","Index.ink", "__ink_37");
	text_69.setAttr("style", "color:#888;;letter-spacing:5px;;font-size:12px;;");

	view_60.addChild(text_69);
	view_59.addChild(view_60);
	VNode view_72("div", "", "__ink_38");
		view_72.setAttr("id", "hr");
	view_72.setAttr("style", "height:1px;;background-color:#333;;margin:10px 0;;");

	view_59.addChild(view_72);
	VNode view_74("div", "", "__ink_39");
		view_74.setAttr("id", "terminal-content");
	view_74.setAttr("class",  "terminalcontent");

	VNode view_75("div", "", "__ink_40");
		view_75.setAttr("id", "terminal-line");
	view_75.setAttr("class",  "terminalline");

		VNode text_76("p","1", "__ink_41");
	text_76.setAttr("class",  "terminallineno");

	view_75.addChild(text_76);
	VNode view_78("div", "", "__ink_42");
		view_78.setAttr("id", "terminal-page-line");
	view_78.setAttr("style", "display:flex;;align-items:center;;");

		VNode text_79("p","page", "__ink_43");
	text_79.setAttr("class",  "codeeditor");
	text_79.setAttr("style", "color:rgb(114, 52, 230);;");

	view_78.addChild(text_79);
		VNode text_81("p","('My Index Page') {", "__ink_44");
	text_81.setAttr("class",  "codeeditor");

	view_78.addChild(text_81);
	view_75.addChild(view_78);
	view_74.addChild(view_75);
	VNode view_85("div", "", "__ink_45");
		view_85.setAttr("id", "terminal-line");
	view_85.setAttr("class",  "terminalline");

		VNode text_86("p","2", "__ink_46");
	text_86.setAttr("class",  "terminallineno");

	view_85.addChild(text_86);
	VNode view_88("div", "", "__ink_47");
		view_88.setAttr("id", "terminal-view-line");
	view_88.setAttr("style", "display:flex;;align-items:center;;");

		VNode text_89("p","    view", "__ink_48");
	text_89.setAttr("class",  "codeeditor");
	text_89.setAttr("style", "color:rgb(114, 52, 230);;");

	view_88.addChild(text_89);
		VNode text_91("p","('mydiv', style={", "__ink_49");
	text_91.setAttr("class",  "codeeditor");

	view_88.addChild(text_91);
	view_85.addChild(view_88);
	view_74.addChild(view_85);
	VNode view_95("div", "", "__ink_50");
		view_95.setAttr("id", "terminal-line");
	view_95.setAttr("class",  "terminalline");

		VNode text_96("p","3", "__ink_51");
	text_96.setAttr("class",  "terminallineno");

	view_95.addChild(text_96);
		VNode text_98("p","        'background-color': 'black',", "__ink_52");
	text_98.setAttr("class",  "codeeditor");

	view_95.addChild(text_98);
	view_74.addChild(view_95);
	VNode view_101("div", "", "__ink_53");
		view_101.setAttr("id", "terminal-line");
	view_101.setAttr("class",  "terminalline");

		VNode text_102("p","4", "__ink_54");
	text_102.setAttr("class",  "terminallineno");

	view_101.addChild(text_102);
		VNode text_104("p","        'color': 'white',", "__ink_55");
	text_104.setAttr("class",  "codeeditor");

	view_101.addChild(text_104);
	view_74.addChild(view_101);
	VNode view_107("div", "", "__ink_56");
		view_107.setAttr("id", "terminal-line");
	view_107.setAttr("class",  "terminalline");

		VNode text_108("p","5", "__ink_57");
	text_108.setAttr("class",  "terminallineno");

	view_107.addChild(text_108);
		VNode text_110("p","        'padding': '20px',", "__ink_58");
	text_110.setAttr("class",  "codeeditor");

	view_107.addChild(text_110);
	view_74.addChild(view_107);
	VNode view_113("div", "", "__ink_59");
		view_113.setAttr("id", "terminal-line");
	view_113.setAttr("class",  "terminalline");

		VNode text_114("p","6", "__ink_60");
	text_114.setAttr("class",  "terminallineno");

	view_113.addChild(text_114);
		VNode text_116("p","        'border-radius': '10px;'", "__ink_61");
	text_116.setAttr("class",  "codeeditor");

	view_113.addChild(text_116);
	view_74.addChild(view_113);
	VNode view_119("div", "", "__ink_62");
		view_119.setAttr("id", "terminal-line");
	view_119.setAttr("class",  "terminalline");

		VNode text_120("p","7", "__ink_63");
	text_120.setAttr("class",  "terminallineno");

	view_119.addChild(text_120);
		VNode text_122("p","}) {", "__ink_64");
	text_122.setAttr("class",  "codeeditor");

	view_119.addChild(text_122);
	view_74.addChild(view_119);
	VNode view_125("div", "", "__ink_65");
		view_125.setAttr("id", "terminal-line");
	view_125.setAttr("class",  "terminalline");

		VNode text_126("p","8", "__ink_66");
	text_126.setAttr("class",  "terminallineno");

	view_125.addChild(text_126);
	VNode view_128("div", "", "__ink_67");
		view_128.setAttr("id", "terminal-page-line");
	view_128.setAttr("style", "display:flex;;align-items:center;;");

		VNode text_129("p","        text", "__ink_68");
	text_129.setAttr("class",  "codeeditor");
	text_129.setAttr("style", "color:rgb(114, 52, 230);;");

	view_128.addChild(text_129);
		VNode text_131("p","('Hello World', cls='herotext')", "__ink_69");
	text_131.setAttr("class",  "codeeditor");

	view_128.addChild(text_131);
	view_125.addChild(view_128);
	view_74.addChild(view_125);
	VNode view_135("div", "", "__ink_70");
		view_135.setAttr("id", "terminal-line");
	view_135.setAttr("class",  "terminalline");

		VNode text_136("p","9", "__ink_71");
	text_136.setAttr("class",  "terminallineno");

	view_135.addChild(text_136);
		VNode text_138("p","    }", "__ink_72");
	text_138.setAttr("class",  "codeeditor");

	view_135.addChild(text_138);
	view_74.addChild(view_135);
	view_59.addChild(view_74);
	view_40.addChild(view_59);
	page.addChild(view_40);
		
	VNode view_143("div", "", "__ink_73");
		view_143.setAttr("id", "features");
	view_143.setAttr("class",  "features");

		VNode text_144("p","Features ", "__ink_74");
	text_144.setAttr("style", "font-size:18px;;font-weight:bold;;");

	view_143.addChild(text_144);
	VNode view_146("div", "", "__ink_75");
		view_146.setAttr("id", "feature-list");
	view_146.setAttr("class",  "featurelist");

	VNode view_147("div", "", "__ink_76");
		view_147.setAttr("id", "feature-grp");

	VNode view_148("div", "", "__ink_77");
		view_148.setAttr("id", "feature-item");
	view_148.setAttr("class",  "featureitem");

	VNode view_149("div", "", "__ink_78");
		view_149.setAttr("id", "feature-icon");
	view_149.setAttr("class",  "lni lni-rocket-5");

	view_148.addChild(view_149);
		VNode text_151("p","Blazing Fast Performance", "__ink_79");

	view_148.addChild(text_151);
	view_147.addChild(view_148);
	VNode view_154("div", "", "__ink_80");
		view_154.setAttr("id", "feature-item");
	view_154.setAttr("class",  "featureitem");

	VNode view_155("div", "", "__ink_81");
		view_155.setAttr("id", "feature-icon");
	view_155.setAttr("class",  "lni lni-colour-palette-3");

	view_154.addChild(view_155);
		VNode text_157("p","Rich Component Library", "__ink_82");

	view_154.addChild(text_157);
	view_147.addChild(view_154);
	view_146.addChild(view_147);
	VNode view_161("div", "", "__ink_83");
		view_161.setAttr("id", "feature-grp");

	VNode view_162("div", "", "__ink_84");
		view_162.setAttr("id", "feature-item");
	view_162.setAttr("class",  "featureitem");

	VNode view_163("div", "", "__ink_85");
		view_163.setAttr("id", "feature-icon");
	view_163.setAttr("class",  "lni lni-globe-1");

	view_162.addChild(view_163);
		VNode text_165("p","Cross-Platform Support", "__ink_86");

	view_162.addChild(text_165);
	view_161.addChild(view_162);
	VNode view_168("div", "", "__ink_87");
		view_168.setAttr("id", "feature-item");
	view_168.setAttr("class",  "featureitem");

	VNode view_169("div", "", "__ink_88");
		view_169.setAttr("id", "feature-icon");
	view_169.setAttr("class",  "lni lni-code-1");

	view_168.addChild(view_169);
		VNode text_171("p"," clean and intuitive syntax", "__ink_89");

	view_168.addChild(text_171);
	view_161.addChild(view_168);
	view_146.addChild(view_161);
	view_143.addChild(view_146);
	page.addChild(view_143);
		};
	EM_ASM({
		Module._handleRoute(allocateUTF8(window.location.pathname));
		window.addEventListener("popstate", () => {
		Module._handleRoute(allocateUTF8(window.location.pathname));
		});
	});return 0;
}
