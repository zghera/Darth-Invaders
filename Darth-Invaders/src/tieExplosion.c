const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[25 * 23 * 2 + 1];
} tieExplosionImg = {
  25, 23, 2,
  "\000\010\000\010\000\010\000\020\000\020\000\020\040\030\000\030\000\030\000\040\000\040\000\040\000\040\000(\000("
  "\000\040\000\040\000\030\000\020\000\020\000\020\000\010\000\010\000\010\000\000\000\010\000\010\000\020\000\020\000\030"
  "\000\030\000\030\000\040\000(\040\060aH\201P\040\070bi\310\273\243\201@\060\000\040\000\030\000\030"
  "\000\020\000\020\000\010\000\010\000\010\000\010\000\020\000\020\000\030\000\030\000\040\000(\040\060\241P\243"
  "\221e\302\246\262\212\274\015\356k\365\251\354\346\262@\070\000\040\040\040\000\020"
  "\000\020\000\020\000\010\000\010\000\020\000\020\000\030\040\040\000\040\040\060\040@\346\252\310\343"
  "&\313H\354\247\333I\344\211\354J\365m\376\354\375\006\263\000(\000\040\000\030\000\020"
  "$\071\202\030\000\010\302(\205Q\242\060A(\040(@@\010\324\351\364&\323\006\323(\354"
  "J\365K\365\347\343k\365k\365J\365\207\323\040\060a(\242\060$A\205I\000\020\000\010"
  "\000\020DI\347i\343@\242@\243\211\351\364\351\364\007\354\012\365\354\375\354\365"
  "*\365\247\343d\272\346\312\207\333\205\272a\070a\060\004A\343\070\000\020\000\020\000"
  "\020\000\020\000\030$A\346i\346qG\273\355\365\213\365k\365\014\376\014\366\314\365"
  "k\365\207\333\307\343\247\323\205\272\302`a\070\003I\242\060a(\000\030\000\020\000\020"
  "\000\030\000\030\000(\242\070Da\353\344-\376\253\365\251\354k\365-\376\314\365\213"
  "\375h\364&\323i\354'\313\242P\302H\343H\202\060\040\040\000\030\000\020\000\020\000\030"
  "\000\040\000(@\070\205\252k\365\253\365\211\354F\333\210\364J\365J\365H\354\350"
  "\353\306\302\002y#\201\345\231\242X\242h\040\070\000\040\000\030\000\030\000\020\000\030\000\040"
  "\000(`@\251\344\012\365\014\376\251\364f\343H\364(\354f\333\350\343\307\343\004"
  "\252\203\221\003y#\221\342\200C\231\344\241\040\070\000\040\000\030\000\020\000\030\000\040"
  "\000(\005z\354\365\213\365\251\354f\353\246\353\307\353\007\354\251\364\207\333"
  "\211\344\207\323\306\312c\231C\221C\221c\231\010\364\346\302\040(\040\040\000\020"
  "\000\030\000\040\000(\345y\251\364\214\365G\333\247\353\307\363\206\353\311\364\213"
  "\365\251\364\006\273l\345\203\231c\241\"\211\002\211\"\211g\343\246\262\000(\000"
  "\030\000\020\000\030\040\040\302@\344\221\350\353\251\354(\354\347\363\347\363\347"
  "\363\247\333\311\364k\365\211\344\010\344\247\343F\333\344\251\302p\202`\002"
  "i\040\060\000\040\000\030\000\020\000\030\000\040\000(@@\305\312F\333F\333\307\363\347\363\006"
  "\333B\221\251\354\211\354\213\365\352\364h\364H\364%\262C\221#\211\302P\000"
  "\040\040\040\000\030\000\020\000\020\000\030\000\040\000(\241P\001\201\301p\303\241\345\332d\312"
  "\305\302J\365\314\365\216\376\351\364\247\343\245\312\"\211C\221\245\251"
  "\201H@(\000\030\000\020\000\020\000\020\000\030\000\030\000\040\000(\040\060@\070@@\245\272\251\354"
  "\355\365\014\366m\366\316\376\012\365\247\343G\343c\211\301pdyDQ$IA\040\000\020"
  "\000\020\000\020\000\020\000\030\000\030\000\040\000(\040(\040\060`H\343\221\352\344\216\376m\376"
  "M\376\012\365\247\343\245\322\306\322#yA\070\040\040\040\040A\030\000\020\000\020\000\020"
  "\000\020\000\020\000\030\000\030\040\040\000\040\040\060\040\060@@ByI\324\254\365k\365&\313#\201"
  "\344\261\244\221A@\040\060\040\040\000\030\040\020\000\020\000\010\000\010\000\020\000\020\000\020\000"
  "\030\000\030\000\030\000\040\000(A\070\040\070`@F\222\004\212aHb@\343P\202@a\070\000\040\000\030"
  "\000\020\000\020\000\020\000\010\000\010\000\010\000\020\000\020\000\020\000\020\000\030\000\030\000\040A(\040(a"
  "\060DQ!\060\040\060A\060\201\060\343@a(\000\030\000\020\000\020\000\020\000\010\000\010\000\010\000\010"
  "\000\010\000\020\000\020\000\020\000\020\000\030\000\030\000\030A(A(\000\040\040(\040\040\000\030A\040DI\242"
  "\060\000\020\000\020\000\010\000\010\000\010\000\010\000\000\000\010\000\010\000\010\000\010\000\020\000\020\000\020"
  "\000\020A\030\000\030\000\030\000\030\000\030\040\030\000\020\000\020a\030\206I\000\020\000\020\000\010\000\010"
  "\000\010\000\000\000\000\000\000\000\010\000\010\000\010\000\010\000\010\000\010\000\020\000\020\000\020\000\020\000\030"
  "\000\020\000\020\000\020\000\020\000\020\000\010\000\010\000\010\000\010\000\000\000\000\000\000",
};
