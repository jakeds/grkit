#ifndef SOFT_STATE_H
#define SOFT_STATE_H
/* One-time Soft.15 -> modular Soft migration. New-format state never passes
   through this conversion. The original card state file is never changed. */
static void soft_state_put(struct state *s,u32 key,u32 value) {if(s->pairs<STATE_PAIRS){s->ids[s->pairs]=key;s->values[s->pairs++]=value;}}
static u32 soft_strength(u32 code,u32 diffusion) {static const u8 units[13]={0,4,8,12,1,2,3,5,6,7,9,10,11};if(code>=33&&code<=52)return code-32;u32 v=code<13?units[code]:0;return diffusion?((v<=8?v:2*v-8)*20+8)/16:(v*20+6)/12;}
static u32 soft_grain_percent(u32 code) {static const u8 p[12]={0,5,10,20,2,4,6,8,12,14,16,18};return code>=33&&code<=52?code-32:code<12?p[code]:0;}
static u32 soft_crop(u32 code) {static const u8 v[8]={0,1,2,3,6,7,8,5};return code<8?v[code]:0;}
static u32 soft_split_preset(u32 code) {static const u8 family[17]={0,6,6,7,7,5,8,6,6,7,7,5,8,1,2,3,4};return code<17?family[code]:0;}
static int soft_old_colour(u32 packed,u32 upper,u32 range,u32 sat) {static const signed char a[8]={0,-25,-50,-75,25,50,-10,10},b[8]={0,-20,-10,10,20,-5,5,-15};if(!(packed&(1U<<28)))return sat?a[(packed>>(range*3))&7]:b[(packed>>(range*3))&7];u32 c=sat?(range<4?(packed>>(range*5))&31:(upper>>((range-4)*5))&31):(packed>>(range*4))&15;if(c>(sat?25U:8U))return 0;return c<=(sat?15U:4U)?-(int)c*5:(int)(c-(sat?15U:4U))*5;}
static u32 soft_edit_value(int value,u32 sat){return value<0?(u32)(value/5)+(sat?16:5):value>0?(u32)(value/5)+(sat?15:4):0;}
static void soft_state_migrate(struct state *s,const u32 *from,const u32 *to,u32 count,u32 parameter_base) {
 u32 sat=0,upper=0,hue=0,light=0,date=0,split=0,strength=0,value;
 state_value(s,402024069U /* colour-saturation.value */,&sat);
 state_value(s,692328237U /* colour-saturation.upper */,&upper);
 state_value(s,183647458U /* colour-hue.value */,&hue);
 state_value(s,449697617U /* colour-brightness.value */,&light);
 state_value(s,541741532U /* date-stamp.style */,&date);
 state_value(s,5749190U /* split-toning.value */,&split);
 state_value(s,682727739U /* split-toning.strength */,&strength);
 u32 original=s->pairs,kept=0;for(u32 p=0;p<original;++p){u32 id=s->ids[p];value=s->values[p];u32 target=0;
 if(id==495043802U /* image-control.effect */){target=861965461U /* CORE.TONE */;value=value;}
 else if(id==684634731U /* framing.ratio */){target=484706023U /* CORE.ASPECT */;value=soft_crop(value);}
 else if(id==161811731U /* guides.style */){target=604351797U /* GUIDES.STYLE */;value=value;}
 else if(id==541741532U /* date-stamp.style */){target=157666443U /* DATE.STYLE */;value=value==5||value==6||value==8?6:value==7?5:value;}
 else if(id==957718554U /* date-stamp.look */){target=247126882U /* DATE.LOOK */;value=value;}
 else if(id==733922165U /* grain.level */){target=790270076U /* CORE.GRAIN */;value=soft_grain_percent(value);}
 else if(id==302107150U /* grain.boost */){target=522078347U /* CORE.GRAIN_BOOST */;value=value;}
 else if(id==319168527U /* grain.pattern */){target=803868407U /* CORE.GRAIN_PATTERN */;value=value;}
 else if(id==851796196U /* bloom.level */){target=242585289U /* SOFTLITE.BLOOM */;value=soft_strength(value,1);}
 else if(id==172354952U /* halation.level */){target=247906215U /* SOFTLITE.HALATION */;value=soft_strength(value,1);}
 else if(id==29132195U /* black-floor.value */){target=431023247U /* SOFTTONE.BLACK */;value=soft_strength(value,0);}
 else if(id==493532345U /* highlight-rolloff.value */){target=629749719U /* SOFTTONE.ROLLOFF */;value=soft_strength(value,0);}
 else if(id==357688225U /* colour-range.value */){target=90810126U /* SOFTCOL.RANGE */;value=value;}
 else if(id==5749190U /* split-toning.value */){target=47138618U /* SOFTSPLT.PRESET */;value=soft_split_preset(value);}
 else if(id==682727739U /* split-toning.strength */){target=103278364U /* SOFTSPLT.STRENGTH */;value=value?value-1:0;}
 else {for(u32 k=0;k<count;++k)if(id==from[k]){target=to[k];break;}if(!target&&id>=parameter_base&&id-parameter_base<=32767U)target=id;}if(target){s->ids[kept]=target;s->values[kept++]=value;}}s->pairs=kept;
 if(date==5||date==6)soft_state_put(s,247126882U /* DATE.LOOK */,date==5?3:4);
 if(split&&!strength){static const u8 p[17]={5,10,20,10,20,20,20,5,15,5,15,10,10,20,20,20,20};soft_state_put(s,103278364U /* SOFTSPLT.STRENGTH */,split<17?p[split]:0);}
 soft_state_put(s,320412812U /* SOFTCOL.SAT0 */,soft_edit_value(soft_old_colour(sat,upper,0,1),1));
 soft_state_put(s,168132270U /* SOFTCOL.HUE0 */,soft_edit_value(soft_old_colour(hue,upper,0,0),0));
 soft_state_put(s,61056144U /* SOFTCOL.LIGHT0 */,soft_edit_value(soft_old_colour(light,upper,0,0),0));
 soft_state_put(s,249452570U /* SOFTCOL.SAT1 */,soft_edit_value(soft_old_colour(sat,upper,1,1),1));
 soft_state_put(s,849836088U /* SOFTCOL.HUE1 */,soft_edit_value(soft_old_colour(hue,upper,1,0),0));
 soft_state_put(s,211719174U /* SOFTCOL.LIGHT1 */,soft_edit_value(soft_old_colour(light,upper,1,0),0));
 soft_state_put(s,681969568U /* SOFTCOL.SAT2 */,soft_edit_value(soft_old_colour(sat,upper,2,1),1));
 soft_state_put(s,880317826U /* SOFTCOL.HUE2 */,soft_edit_value(soft_old_colour(hue,upper,2,0),0));
 soft_state_put(s,509587900U /* SOFTCOL.LIGHT2 */,soft_edit_value(soft_old_colour(light,upper,2,0),0));
 soft_state_put(s,323477814U /* SOFTCOL.SAT3 */,soft_edit_value(soft_old_colour(sat,upper,3,1),1));
 soft_state_put(s,702069524U /* SOFTCOL.HUE3 */,soft_edit_value(soft_old_colour(hue,upper,3,0),0));
 soft_state_put(s,788638506U /* SOFTCOL.LIGHT3 */,soft_edit_value(soft_old_colour(light,upper,3,0),0));
 soft_state_put(s,367911061U /* SOFTCOL.SAT4 */,soft_edit_value(soft_old_colour(sat,upper,4,1),1));
 soft_state_put(s,120767159U /* SOFTCOL.HUE4 */,soft_edit_value(soft_old_colour(hue,upper,4,0),0));
 soft_state_put(s,945007241U /* SOFTCOL.LIGHT4 */,soft_edit_value(soft_old_colour(light,upper,4,0),0));
 soft_state_put(s,196680707U /* SOFTCOL.SAT5 */,soft_edit_value(soft_old_colour(sat,upper,5,1),1));
 soft_state_put(s,902478881U /* SOFTCOL.HUE5 */,soft_edit_value(soft_old_colour(hue,upper,5,0),0));
 soft_state_put(s,330944543U /* SOFTCOL.LIGHT5 */,soft_edit_value(soft_old_colour(light,upper,5,0),0));
 soft_state_put(s,663899065U /* SOFTCOL.SAT6 */,soft_edit_value(soft_old_colour(sat,upper,6,1),1));
 soft_state_put(s,898521499U /* SOFTCOL.HUE6 */,soft_edit_value(soft_old_colour(hue,upper,6,0),0));
 soft_state_put(s,596471205U /* SOFTCOL.LIGHT6 */,soft_edit_value(soft_old_colour(light,upper,6,0),0));
}
#endif
