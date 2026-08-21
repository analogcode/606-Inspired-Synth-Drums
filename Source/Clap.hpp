#pragma once

// Super 606 Clap
// Copyright (c) 2026 Matthew Fecher (AnalogMatthew)

#include "SynthDrumCommon.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace SynthDrums606 {
namespace ClapDetail {

// I keep the fitted timing and sample-rate conversion in double so the burst
// edges do not move at the other sample rates
static constexpr double kReferenceSampleRate = 44100.0;
static constexpr double kMinimumSupportedSampleRate = 8000.0;
static constexpr double kMaximumSupportedSampleRate = 192000.0;
static constexpr double kClapPi = 3.14159265358979323846264338327950288;
static constexpr double kT60ToTau = 6.907755278982137;
static constexpr double kVoiceDurationSeconds = 0.301;
static constexpr double kOutputTrim = 0.1793169072680733;
static constexpr double kSaturationThreshold = 3.0;
static constexpr double kAirHighpassHz = 3000.0;
static constexpr double kAirDensityThreshold = 0.75;

static constexpr std::size_t kColorTapCount = 192;
static constexpr std::array<double, 4> kShortOnsets = {{
    0.0, 0.010737, 0.021735, 0.032778
}};
static constexpr double kTerminalOnset = 0.03278;
static constexpr double kSnapOnset = 0.032778;
static constexpr double kFoundationOnset = 0.165;
static constexpr double kTerminalFloorOnset = 0.250;

// Four short minimum-phase FIRs fitted from hardware section spectra shape one
// shared generated-noise stream. Burst timing and envelopes are synthesized
// below.
static constexpr std::array<float, kColorTapCount> kOpeningColor = {{
    1.7299991247e-01f, 3.8520469294e-01f, 3.6505443856e-01f, 3.0560679220e-01f,
    2.2452826026e-01f, 1.3908714920e-01f, 9.1927071889e-02f, 1.4103916149e-02f,
    -3.7942942562e-02f, -8.1746751873e-02f, -1.0526792061e-01f, -1.2205764226e-01f,
    -1.4334556915e-01f, -1.5716007865e-01f, -1.7842421430e-01f, -2.0677230638e-01f,
    -2.2379942677e-01f, -2.1503897132e-01f, -1.8576203109e-01f, -1.5083509179e-01f,
    -1.4098250958e-01f, -1.2832528888e-01f, -9.3813025596e-02f, -6.0017450617e-02f,
    -3.1084397093e-02f, -9.4701167885e-03f, -7.7901199547e-03f, 7.1781049559e-03f,
    3.6756277094e-02f, 4.5058386570e-02f, 5.3116535332e-02f, 6.3101344941e-02f,
    7.1431644979e-02f, 7.8671795335e-02f, 6.8710976373e-02f, 5.4966574939e-02f,
    5.4009053330e-02f, 5.7502796101e-02f, 7.0540598859e-02f, 7.3996216522e-02f,
    7.1787348948e-02f, 7.4131139235e-02f, 5.9788514933e-02f, 5.1424890013e-02f,
    4.7546079593e-02f, 4.4829685002e-02f, 3.3747960232e-02f, 2.0824402508e-02f,
    2.4901738822e-02f, 3.5475715459e-02f, 2.5897916476e-02f, -4.5427778201e-03f,
    -1.9814509444e-02f, -4.7212428528e-02f, -6.5262464268e-02f, -7.1616739457e-02f,
    -9.8859981673e-02f, -1.0089199331e-01f, -7.2349242843e-02f, -5.3079429834e-02f,
    -5.2444421821e-02f, -5.2766118080e-02f, -5.5103536933e-02f, -4.3079985895e-02f,
    -2.2298520083e-02f, -1.5277499429e-02f, -2.0432668089e-03f, 3.2518791317e-03f,
    -2.1290429544e-03f, 4.4082757834e-03f, 3.8510594680e-03f, 9.2288308501e-04f,
    1.4605638148e-02f, 2.5834777747e-02f, 3.2312073687e-02f, 4.1136814026e-02f,
    4.3009171598e-02f, 3.5981571251e-02f, 2.5953073495e-02f, 1.8527830369e-02f,
    1.3205486210e-02f, 7.1091980192e-03f, 1.0396742207e-02f, 2.0931430046e-02f,
    3.1227821414e-02f, 4.9580991890e-02f, 5.4667524262e-02f, 4.5332436846e-02f,
    3.1082430763e-02f, 1.1505544386e-02f, 4.0277029028e-03f, -5.4587185574e-03f,
    -1.3768563198e-02f, -1.4379626752e-02f, -1.7033045254e-02f, -1.8838404111e-02f,
    -2.0550629544e-02f, -2.1246278508e-02f, -2.6315479921e-02f, -2.9134645063e-02f,
    -3.1006974644e-02f, -3.1152424500e-02f, -2.1434461031e-02f, -2.0322271955e-02f,
    -2.4219789670e-02f, -1.9885234655e-02f, -1.1013383269e-02f, 7.9772630186e-03f,
    2.0226216757e-02f, 9.8359903528e-03f, -2.6826299890e-03f, -7.5124332925e-03f,
    -1.0426829419e-02f, -1.7244872720e-02f, -2.2549015083e-02f, -1.5310424013e-02f,
    -3.7725572308e-03f, -6.8477817438e-04f, -2.7986644382e-03f, -1.7004582564e-03f,
    6.2946059616e-03f, 9.2504519056e-03f, 6.7589491506e-03f, 1.3722494372e-02f,
    1.9039073252e-02f, 2.2172265663e-02f, 2.8985564843e-02f, 3.0641273329e-02f,
    2.8861809496e-02f, 2.7316314986e-02f, 2.0445908089e-02f, 1.3581485055e-02f,
    1.5115719828e-02f, 1.5888362331e-02f, 1.0698867310e-02f, 2.8339802856e-03f,
    -3.2350045727e-03f, -8.5771050159e-03f, -1.3926923145e-02f, -1.8129481739e-02f,
    -2.1631097326e-02f, -2.1335178464e-02f, -1.8788879538e-02f, -1.4653481790e-02f,
    -8.9357048749e-03f, -6.2253231116e-03f, -7.2436394306e-03f, -9.2222284343e-03f,
    -9.9267520476e-03f, -1.0635525159e-02f, -1.0652959172e-02f, -8.2489791203e-03f,
    -9.5926427013e-03f, -9.7875005383e-03f, -3.8607442057e-03f, 3.8359976765e-04f,
    4.0924949739e-03f, 4.2494315822e-03f, 2.5127958410e-03f, 6.7434326157e-03f,
    8.9475745172e-03f, 6.7927039824e-03f, 4.4936897603e-03f, 2.5254862378e-03f,
    3.6229308800e-03f, 7.0011716518e-03f, 8.7319443032e-03f, 8.9431225322e-03f,
    8.5529789044e-03f, 7.2768236186e-03f, 6.1263372792e-03f, 5.2646360283e-03f,
    3.9868230466e-03f, 3.0749426049e-03f, 2.9882810754e-03f, 2.4603606808e-03f,
    1.0739149034e-03f, -1.9589900060e-04f, -1.4336608958e-03f, -2.2916974318e-03f,
    -2.2258638497e-03f, -1.8234234952e-03f, -1.2839692385e-03f, -9.3576372805e-04f,
    -7.9497181702e-04f, -6.1005499408e-04f, -4.0669433945e-04f, -2.1851935469e-04f,
    -8.8834898005e-05f, -2.5824787821e-05f, -4.9454475024e-06f, -0.0000000000e+00f,
}};

static constexpr std::array<float, kColorTapCount> kHandoffColor = {{
    1.8875619402e-01f, 3.9532535099e-01f, 3.1052115012e-01f, 2.0417681294e-01f,
    1.2327780667e-01f, 5.3936258901e-02f, 2.2253809872e-02f, -4.6593291093e-02f,
    -1.0347847143e-01f, -1.6319263581e-01f, -1.8598616783e-01f, -1.9691378438e-01f,
    -1.9998351978e-01f, -1.9882124185e-01f, -1.9398388684e-01f, -1.7466283449e-01f,
    -1.7442871397e-01f, -1.6680812656e-01f, -1.4932833745e-01f, -1.1161678039e-01f,
    -7.9639914597e-02f, -6.0494803220e-02f, -4.3097509189e-02f, -3.7946681895e-02f,
    -2.7856113623e-02f, -4.0528471275e-03f, 2.4570897462e-02f, 5.1935184672e-02f,
    7.3481992541e-02f, 8.0365700782e-02f, 9.3774064874e-02f, 1.1429755485e-01f,
    1.2814304655e-01f, 1.3704431024e-01f, 1.3654880269e-01f, 1.3384377070e-01f,
    1.2377069562e-01f, 1.1334862119e-01f, 1.1755749462e-01f, 1.1280667005e-01f,
    8.8223363515e-02f, 6.0171909313e-02f, 4.3882991093e-02f, 3.1759968057e-02f,
    1.0557516928e-02f, 6.3407390303e-03f, 2.2863515741e-02f, 1.7511511564e-02f,
    -1.1691636252e-02f, -4.0678974631e-02f, -5.9415059590e-02f, -6.6718975025e-02f,
    -6.6513089303e-02f, -5.9587464418e-02f, -6.1848135339e-02f, -7.0400122558e-02f,
    -7.4952977630e-02f, -8.0457044658e-02f, -7.7644015860e-02f, -6.1188557650e-02f,
    -4.1233682302e-02f, -3.3080606371e-02f, -3.5374527778e-02f, -4.1106987629e-02f,
    -4.0476444817e-02f, -3.6943129798e-02f, -3.7512208948e-02f, -3.9119038538e-02f,
    -3.6975340594e-02f, -1.4263153464e-02f, 8.0800135068e-03f, 1.7602733217e-02f,
    2.2936607116e-02f, 2.2241988585e-02f, 2.4327434638e-02f, 3.5654667183e-02f,
    5.1179527021e-02f, 5.0305826316e-02f, 3.9597608314e-02f, 3.1995287216e-02f,
    2.8936693434e-02f, 2.9734677245e-02f, 2.6427212406e-02f, 2.7868867792e-02f,
    3.0683092212e-02f, 2.8941365711e-02f, 2.5365597524e-02f, 1.6343393416e-02f,
    8.8617725898e-03f, 4.0539345935e-03f, 3.2316595056e-04f, -5.5273412411e-04f,
    3.0139986806e-04f, 6.4484454176e-04f, -2.5984268478e-03f, -7.3756520558e-03f,
    -1.7241810261e-02f, -1.6335407228e-02f, -7.2853057099e-03f, -1.0271613210e-02f,
    -8.2024109995e-03f, -5.8863433646e-03f, -1.0774098558e-02f, -1.1499064632e-02f,
    -1.0340341879e-03f, 1.9238286584e-02f, 2.4192210920e-02f, 1.0727129830e-02f,
    2.4777733322e-03f, 1.0215582235e-03f, 2.4551051176e-03f, 9.3889916913e-03f,
    4.0282178255e-03f, -8.7561536902e-03f, -4.9092698784e-03f, -8.6139566513e-04f,
    -9.5470480259e-03f, -2.1571357383e-02f, -2.8191072376e-02f, -2.7298650306e-02f,
    -2.4646025014e-02f, -2.2188370626e-02f, -2.2663653592e-02f, -1.6921077700e-02f,
    -5.8783140947e-03f, -3.0692155482e-03f, -2.5810634026e-03f, -3.0547516211e-03f,
    -2.0676771688e-03f, -2.2964005711e-03f, -7.0261418575e-03f, -8.6823902664e-03f,
    -1.3690616010e-02f, -1.5594866246e-02f, -1.5848685375e-02f, -1.4647070009e-02f,
    2.1477154053e-04f, 1.7655075723e-02f, 3.0585570046e-02f, 3.5469789746e-02f,
    3.3094484497e-02f, 2.6992796577e-02f, 2.6424372263e-02f, 3.1620824835e-02f,
    3.1452315987e-02f, 2.4288905438e-02f, 2.1992004564e-02f, 2.9756968347e-02f,
    2.6089126997e-02f, 2.1342070492e-02f, 2.2254626751e-02f, 1.9769410909e-02f,
    1.6155106566e-02f, 4.3064050354e-03f, -6.2038642358e-03f, -1.0095456038e-02f,
    -1.3423823436e-02f, -1.9278293978e-02f, -1.9673820252e-02f, -1.1910960372e-02f,
    -1.3594786202e-02f, -2.1142244028e-02f, -2.7204330424e-02f, -3.1068176630e-02f,
    -2.7894902200e-02f, -2.7153380416e-02f, -2.8041008611e-02f, -2.4219212736e-02f,
    -2.0634875707e-02f, -1.6187012168e-02f, -1.0479424511e-02f, -7.2663527681e-03f,
    -5.6374416729e-03f, -3.7422698206e-03f, -1.4163680254e-03f, 3.3928004566e-04f,
    2.3420011879e-03f, 4.7162152723e-03f, 5.4279575118e-03f, 5.5645376352e-03f,
    5.5513861468e-03f, 4.3937996309e-03f, 3.2511581219e-03f, 2.9783092063e-03f,
    2.7951769845e-03f, 2.2479299804e-03f, 1.3995877132e-03f, 6.9904864455e-04f,
    3.6704890385e-04f, 1.7713908110e-04f, 3.7858337451e-05f, 0.0000000000e+00f,
}};

static constexpr std::array<float, kColorTapCount> kFastColor = {{
    1.4644279033e-01f, 3.3251425492e-01f, 3.0859052842e-01f, 2.3850248802e-01f,
    1.7106916088e-01f, 8.8562854779e-02f, 3.1893734688e-02f, -3.0031178833e-02f,
    -7.9945965465e-02f, -1.3244572685e-01f, -1.6362719173e-01f, -1.7710008278e-01f,
    -1.9040712613e-01f, -2.0589490809e-01f, -2.1188769343e-01f, -2.0689380767e-01f,
    -2.0043829364e-01f, -1.8881694375e-01f, -1.7639896033e-01f, -1.5042825626e-01f,
    -1.1744597235e-01f, -7.8919664332e-02f, -4.4104005328e-02f, -3.8904541670e-03f,
    3.0533000837e-02f, 4.4945801627e-02f, 6.5836836767e-02f, 8.2453527556e-02f,
    9.2875174988e-02f, 9.7217019887e-02f, 1.0303531878e-01f, 1.0783131664e-01f,
    1.0028010263e-01f, 9.9973070631e-02f, 9.5708973060e-02f, 8.3961725978e-02f,
    8.0039703641e-02f, 8.8430083766e-02f, 9.5913529808e-02f, 8.7543081064e-02f,
    7.9121999752e-02f, 7.5578302503e-02f, 7.1182878368e-02f, 5.7252560998e-02f,
    3.9309422778e-02f, 2.5171764280e-02f, 4.4653573320e-03f, -1.1864384777e-02f,
    -3.0366262986e-02f, -5.5890672791e-02f, -7.5085014848e-02f, -8.6867688815e-02f,
    -8.8778224349e-02f, -9.0584711493e-02f, -9.2215571744e-02f, -9.1266542927e-02f,
    -9.2624401986e-02f, -9.6245312917e-02f, -8.9648770150e-02f, -6.9226921724e-02f,
    -5.5988772723e-02f, -4.4530985568e-02f, -2.6720085666e-02f, -7.9709798946e-03f,
    9.1258579198e-03f, 2.3305216710e-02f, 3.2412006527e-02f, 3.3089622507e-02f,
    3.5608404131e-02f, 3.9076460601e-02f, 3.4604297928e-02f, 3.3155923189e-02f,
    4.1606745316e-02f, 4.6396513320e-02f, 4.8867472567e-02f, 5.6108334898e-02f,
    6.0791044239e-02f, 6.1414992382e-02f, 5.3634383802e-02f, 4.0484233121e-02f,
    2.5334577443e-02f, 6.9592828718e-03f, -5.5072231193e-03f, -1.3580403470e-02f,
    -2.4465813753e-02f, -3.0377069583e-02f, -2.5157539555e-02f, -2.2014305929e-02f,
    -1.8436730819e-02f, -1.3259652274e-02f, -1.5716725316e-02f, -1.5201767541e-02f,
    -1.0464498135e-02f, -1.0328027209e-02f, -1.3266340203e-02f, -9.3307773966e-03f,
    -3.3698780013e-03f, 2.2331068473e-03f, 8.3534615478e-03f, 1.1517625483e-02f,
    1.1225234035e-02f, 5.6612264261e-03f, 8.4687115795e-03f, 1.1962385205e-02f,
    5.6750515076e-03f, -3.4196079220e-03f, -1.1423524889e-02f, -1.5239401905e-02f,
    -1.6309600843e-02f, -1.7943625745e-02f, -1.8068378795e-02f, -8.8903004155e-03f,
    -3.5731864575e-03f, -5.5923071192e-03f, -7.9615682441e-03f, -1.1019803958e-02f,
    -8.3741907120e-03f, -6.0801361338e-03f, -5.9358203047e-03f, -4.4675137155e-03f,
    -5.5096303097e-03f, -7.7904723738e-04f, 1.1686518541e-02f, 1.5498594718e-02f,
    1.1043152991e-02f, 6.8694248426e-03f, 3.9785150408e-03f, 5.6377612554e-03f,
    7.6844872390e-03f, 9.0475647339e-03f, 1.3301744585e-02f, 1.8719059063e-02f,
    2.3454687156e-02f, 2.2704440167e-02f, 2.1347777532e-02f, 2.1712409763e-02f,
    1.7248403430e-02f, 1.1262126584e-02f, 8.4824704213e-03f, 6.0440605959e-03f,
    -1.6710228449e-03f, -4.4042710423e-03f, -1.8068409087e-03f, -3.7550599497e-03f,
    -5.3673556029e-03f, -6.3848212539e-03f, -5.3632438609e-03f, -4.7760217279e-03f,
    -8.7347270831e-03f, -1.2969979200e-02f, -1.4830935442e-02f, -1.3895969576e-02f,
    -1.5001843702e-02f, -1.7324794169e-02f, -1.7296900826e-02f, -1.8139915010e-02f,
    -2.1433352777e-02f, -2.6018132861e-02f, -2.6095548860e-02f, -1.9520000378e-02f,
    -1.1810525169e-02f, -6.4036120259e-03f, -2.0758811410e-03f, 1.4735475877e-03f,
    4.3694484793e-03f, 9.1458877127e-03f, 1.1001737153e-02f, 9.1675550847e-03f,
    8.1882326218e-03f, 8.3674704541e-03f, 9.4950997703e-03f, 9.4993343768e-03f,
    9.3898558781e-03f, 9.0901648646e-03f, 7.2145556756e-03f, 5.2846953263e-03f,
    3.8242149942e-03f, 2.2761975244e-03f, 9.8110836439e-04f, -2.0780521747e-04f,
    -1.5306391481e-03f, -1.6848704170e-03f, -8.8404675258e-04f, -3.0776405227e-04f,
    -2.2786459061e-04f, -1.2461349374e-04f, 5.4600496690e-05f, 5.5594324186e-05f,
    7.2649705392e-06f, -9.3334849094e-06f, -3.7097946147e-06f, -0.0000000000e+00f,
}};

static constexpr std::array<float, kColorTapCount> kLateColor = {{
    8.7130201318e-02f, 2.2155809298e-01f, 2.8348361607e-01f, 2.8969521695e-01f,
    2.5567232669e-01f, 2.0070052321e-01f, 1.3676317259e-01f, 7.4221264546e-02f,
    2.1740344735e-02f, -3.2235552780e-02f, -7.9975832576e-02f, -1.2418344686e-01f,
    -1.5733516311e-01f, -1.8224817432e-01f, -2.0354665656e-01f, -2.1595437326e-01f,
    -2.1840114540e-01f, -2.1408287478e-01f, -2.0775063975e-01f, -1.9588524792e-01f,
    -1.7977876415e-01f, -1.5828956198e-01f, -1.3631974334e-01f, -1.1430055582e-01f,
    -8.9797315789e-02f, -6.2661166421e-02f, -3.5405955397e-02f, -1.3159330961e-02f,
    9.6000203231e-03f, 3.1611400021e-02f, 5.1726064218e-02f, 6.8934579219e-02f,
    8.2731548195e-02f, 9.3974995507e-02f, 1.0207550769e-01f, 1.1138532055e-01f,
    1.1672223674e-01f, 1.1800638420e-01f, 1.1939892180e-01f, 1.1432582188e-01f,
    1.0506392496e-01f, 9.6589559732e-02f, 8.8206046829e-02f, 7.9076695511e-02f,
    6.6358888242e-02f, 5.2976560198e-02f, 4.0920832018e-02f, 2.7033126345e-02f,
    1.2788449284e-02f, -8.3522389302e-04f, -1.4258011163e-02f, -2.7707175256e-02f,
    -4.0898501891e-02f, -5.1405484331e-02f, -5.7644255084e-02f, -6.1241170245e-02f,
    -6.4352140841e-02f, -6.6777290497e-02f, -6.8791522233e-02f, -6.8268254727e-02f,
    -6.2136623545e-02f, -4.9844425610e-02f, -3.6728842541e-02f, -2.7176030821e-02f,
    -1.9985779258e-02f, -1.6729785576e-02f, -1.0199577848e-02f, 1.1963622227e-03f,
    1.0580382277e-02f, 1.7103609343e-02f, 2.0289546288e-02f, 2.2227453956e-02f,
    2.3310296858e-02f, 2.5875142251e-02f, 3.0148347540e-02f, 3.2651095324e-02f,
    3.4763920863e-02f, 3.5975117165e-02f, 3.4584726143e-02f, 3.1687552408e-02f,
    2.8861034451e-02f, 2.3490087772e-02f, 1.6815260403e-02f, 1.0886991352e-02f,
    5.1574091197e-03f, 1.0787372940e-03f, -4.2088313830e-03f, -9.1105071966e-03f,
    -1.2059160842e-02f, -1.4331597218e-02f, -1.4107396181e-02f, -1.4870141096e-02f,
    -1.8241726463e-02f, -2.1682008998e-02f, -2.4036347026e-02f, -2.3494730691e-02f,
    -2.0688706205e-02f, -1.8326889810e-02f, -1.5230415289e-02f, -1.1777225703e-02f,
    -7.1524386733e-03f, -1.4101943252e-03f, 1.5418741362e-03f, 3.6459285113e-03f,
    6.2476352278e-03f, 7.7223464474e-03f, 8.3337695167e-03f, 9.4597559187e-03f,
    1.0568134465e-02f, 9.2203000562e-03f, 7.7240427661e-03f, 8.7404367115e-03f,
    9.0389128719e-03f, 7.5681684818e-03f, 6.3264442002e-03f, 5.4600726174e-03f,
    3.8208311955e-03f, 2.8758134510e-03f, 4.2005461535e-03f, 5.4062053862e-03f,
    6.4444727542e-03f, 6.8311808173e-03f, 5.4200081292e-03f, 3.6673254723e-03f,
    9.4138177350e-04f, -1.2776751178e-03f, -1.3783768568e-03f, 2.5254776593e-04f,
    2.4141877170e-03f, 2.7787826027e-03f, 8.1125846429e-04f, -2.2762711770e-03f,
    -4.2076426997e-03f, -5.3592031942e-03f, -7.0290615545e-03f, -7.5847080386e-03f,
    -6.6053000721e-03f, -5.2488755389e-03f, -5.3682239908e-03f, -5.9946908463e-03f,
    -6.0427003775e-03f, -7.1768089447e-03f, -7.1268693491e-03f, -4.6158791520e-03f,
    -2.6794016852e-03f, -1.3168045105e-03f, 1.0162617713e-03f, 2.7650499789e-03f,
    3.7682566833e-03f, 4.4813817320e-03f, 3.7970185695e-03f, 1.9352859909e-03f,
    2.8837791595e-04f, 5.9387703962e-04f, 1.7123794523e-03f, 2.3434977281e-03f,
    2.5523219511e-03f, 1.3922337427e-03f, -1.0847044549e-03f, -3.2997248024e-03f,
    -3.4236017940e-03f, -1.1795445111e-03f, 1.0515971477e-03f, 9.3306626293e-04f,
    8.4823736794e-04f, 1.4738633314e-03f, 1.7278435695e-03f, 1.9500228298e-03f,
    1.9098723026e-03f, 1.9834121276e-03f, 2.1886978555e-03f, 2.2751726702e-03f,
    2.3278500034e-03f, 2.3505952985e-03f, 1.7926370794e-03f, 9.5787423866e-04f,
    4.2225305007e-04f, 2.5186489566e-04f, 9.1255405584e-05f, -3.9853992958e-04f,
    -7.7767560106e-04f, -8.7674023069e-04f, -7.8006074845e-04f, -5.7859495911e-04f,
    -4.1427615131e-04f, -2.7300701604e-04f, -1.7115147834e-04f, -9.2326998995e-05f,
    -4.3627117585e-05f, -2.0987719357e-05f, -5.3857387243e-06f, -0.0000000000e+00f,
}};

class Random {
public:
    void seed(uint32_t value) {
        state_ = value == 0u ? 0x6D2B79F5u : value;
    }

    float gaussianish() {
        double sum = 0.0;
        for (int draw = 0; draw < 6; ++draw) {
            sum += bipolar();
        }
        return static_cast<float>(sum * 0.70710678118654752440);
    }

private:
    double bipolar() {
        uint32_t value = state_;
        value ^= value << 13;
        value ^= value >> 17;
        value ^= value << 5;
        state_ = value;
        return static_cast<double>(value >> 8)
            * (2.0 / 16777215.0) - 1.0;
    }

    uint32_t state_ = 0x6D2B79F5u;
};

template <std::size_t TapCount>
class CompressedSpectralCurve {
public:
    void configure(const std::array<float, TapCount> &source,
                   float sourceStep) {
        const float step = std::max(sourceStep, 1.0f);
        tapCount_ = std::max<std::size_t>(1, std::min(
            static_cast<std::size_t>(std::ceil(
                static_cast<float>(TapCount) / step)), TapCount));
        if (step == 1.0f) {
            coefficients_ = source;
            tapCount_ = TapCount;
            return;
        }

        double rawEnergy = 0.0;
        double sourceEnergy = 0.0;
        for (float value : source) {
            sourceEnergy += value * value;
        }
        for (std::size_t tap = 0; tap < tapCount_; ++tap) {
            coefficients_[tap] = cubicSample(
                source, static_cast<float>(tap) * step);
            rawEnergy += coefficients_[tap] * coefficients_[tap];
        }
        const float scale = static_cast<float>(std::sqrt(
            sourceEnergy / std::max(rawEnergy, 1.0e-20)));
        for (std::size_t tap = 0; tap < tapCount_; ++tap) {
            coefficients_[tap] *= scale;
        }
        std::fill(coefficients_.begin() + tapCount_,
                  coefficients_.end(), 0.0f);
    }

    float coefficient(std::size_t tap) const {
        return coefficients_[tap];
    }

    std::size_t tapCount() const { return tapCount_; }

private:
    static float sourceSample(const std::array<float, TapCount> &source,
                              int index) {
        return index >= 0 && index < static_cast<int>(TapCount)
            ? source[static_cast<std::size_t>(index)] : 0.0f;
    }

    static float cubicSample(const std::array<float, TapCount> &source,
                             float position) {
        const int index = static_cast<int>(std::floor(position));
        const float t = position - static_cast<float>(index);
        const float p0 = sourceSample(source, index - 1);
        const float p1 = sourceSample(source, index);
        const float p2 = sourceSample(source, index + 1);
        const float p3 = sourceSample(source, index + 2);
        const float t2 = t * t;
        const float t3 = t2 * t;
        return 0.5f * (2.0f * p1 + (-p0 + p2) * t
            + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2
            + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
    }

    std::array<float, TapCount> coefficients_ = {{}};
    std::size_t tapCount_ = TapCount;
};

struct ColorFrame {
    float opening = 0.0f;
    float handoff = 0.0f;
    float fast = 0.0f;
    float late = 0.0f;
};

// Keep the dry body beside the full clap so Noise can change the tail without
// starting another random stream
struct CoreFrame {
    float full = 0.0f;
    float dryBody = 0.0f;
};

// All four filters get the same noise. Separate noise here made the color
// changes sound like crossfades instead of one clap opening up
class CorrelatedColorBank {
public:
    void configure(float sourceStep) {
        opening_.configure(kOpeningColor, sourceStep);
        handoff_.configure(kHandoffColor, sourceStep);
        fast_.configure(kFastColor, sourceStep);
        late_.configure(kLateColor, sourceStep);
        tapCount_ = opening_.tapCount();
        history_.fill(0.0f);
        position_ = 0;
    }

    void prewarm(Random &random) {
        // Tune changes the filter length, but it should not change the random
        // starting point
        for (std::size_t sample = 0; sample < kColorTapCount; ++sample) {
            push(random.gaussianish());
        }
    }

    ColorFrame process(float noise) {
        push(noise);
        ColorFrame output;
        for (std::size_t tap = 0; tap < tapCount_; ++tap) {
            const std::size_t index = (position_ + tapCount_ - 1 - tap)
                % tapCount_;
            const float sample = history_[index];
            output.opening += opening_.coefficient(tap) * sample;
            output.handoff += handoff_.coefficient(tap) * sample;
            output.fast += fast_.coefficient(tap) * sample;
            output.late += late_.coefficient(tap) * sample;
        }
        output.opening = flushDenormal(output.opening);
        output.handoff = flushDenormal(output.handoff);
        output.fast = flushDenormal(output.fast);
        output.late = flushDenormal(output.late);
        return output;
    }

private:
    void push(float sample) {
        history_[position_] = flushDenormal(sample);
        position_ = position_ + 1 == tapCount_ ? 0 : position_ + 1;
    }

    CompressedSpectralCurve<kColorTapCount> opening_;
    CompressedSpectralCurve<kColorTapCount> handoff_;
    CompressedSpectralCurve<kColorTapCount> fast_;
    CompressedSpectralCurve<kColorTapCount> late_;
    std::array<float, kColorTapCount> history_ = {{}};
    std::size_t tapCount_ = kColorTapCount;
    std::size_t position_ = 0;
};

inline double raisedCosineRamp(double time, double start, double end) {
    if (time <= start) {
        return 0.0;
    }
    if (time >= end) {
        return 1.0;
    }
    const double position = (time - start) / (end - start);
    return 0.5 - 0.5 * std::cos(kClapPi * position);
}

inline double raisedSineGate(double time, double onset, double attack,
                             double hold, double t60) {
    const double age = time - onset;
    if (age < 0.0) {
        return 0.0;
    }
    double attackGain = 1.0;
    if (attack > 0.0 && age < attack) {
        const double sine = std::sin(0.5 * kClapPi * age / attack);
        attackGain = sine * sine;
    }
    const double decayAge = std::max(age - attack - hold, 0.0);
    return attackGain * std::exp(
        -kT60ToTau * decayAge / std::max(t60, 1.0e-6));
}

class ClapCore {
public:
    void init(uint32_t seed) {
        random_.seed(seed);
    }

    void trigger(double sampleRate, float decay, float sourceStep,
                 float airSpread = 0.0f) {
        sampleRate_ = sampleRate;
        decay_ = decay;
        airSpread_ = clampf(airSpread, 0.0f, 1.0f);
        colors_.configure(sourceStep);
        colors_.prewarm(random_);
        sampleIndex_ = 0;
    }

    CoreFrame process() {
        const ColorFrame color = colors_.process(random_.gaussianish());
        const double time = static_cast<double>(sampleIndex_) / sampleRate_;
        const double decay = static_cast<double>(decay_);
        const double diffuseDecayScale = 1.0
            + 0.45 * static_cast<double>(airSpread_);

        constexpr std::array<double, 4> levels = {{
            2.00, 1.85, 1.16, 0.46
        }};
        constexpr std::array<double, 4> t60s = {{
            0.0125, 0.014, 0.019, 0.025
        }};

        double shortEnvelope = 0.0;
        for (std::size_t burst = 0; burst < kShortOnsets.size(); ++burst) {
            shortEnvelope += levels[burst] * raisedSineGate(
                time, kShortOnsets[burst],
                burst == 0 ? 0.0 : 0.00016,
                0.00030, t60s[burst] * decay);
        }
        double dryBody = static_cast<double>(color.opening) * shortEnvelope;
        double output = dryBody;

        const double terminalAge = time - kTerminalOnset;
        if (terminalAge >= 0.0) {
            const double terminalAttack = terminalAge < 0.00020
                ? std::pow(std::sin(
                    0.5 * kClapPi * terminalAge / 0.00020), 2.0)
                : 1.0;
            const double terminalDecayAge = std::max(
                terminalAge - 0.00020 - 0.019, 0.0);
            const double terminalEnvelope = 0.72 * terminalAttack * (
                0.955 * std::exp(
                    -kT60ToTau * terminalDecayAge
                        / ((0.225 * decay) * diffuseDecayScale))
                + 0.045 * std::exp(
                    -kT60ToTau * terminalDecayAge
                        / ((0.890 * decay) * diffuseDecayScale)));

            const double fastMix = raisedCosineRamp(time, 0.055, 0.080);
            double terminalColor = (1.0 - fastMix) * color.handoff
                + fastMix * color.fast;
            const double lateMix = raisedCosineRamp(time, 0.110, 0.170);
            terminalColor = (1.0 - lateMix) * terminalColor
                + lateMix * color.late;
            output += terminalEnvelope * terminalColor;
        }

        const double snap = 0.55 * color.opening * raisedSineGate(
            time, kSnapOnset, 0.00012, 0.00035, 0.025 * decay);
        output += snap;
        dryBody += snap;
        // Keep the two late layers tied to Decay so a short clap does not fade
        // out and pop back in
        output += 0.022 * decay * color.late * raisedSineGate(
            time, kFoundationOnset, 0.020, 0.0,
            (0.760 * decay) * diffuseDecayScale);
        output += 0.004 * decay * color.late * raisedSineGate(
            time, kTerminalFloorOnset, 0.015, 0.0,
            (0.760 * decay) * diffuseDecayScale);

        ++sampleIndex_;
        CoreFrame frame;
        frame.full = flushDenormal(static_cast<float>(output));
        frame.dryBody = flushDenormal(static_cast<float>(dryBody));
        return frame;
    }

private:
    double sampleRate_ = kReferenceSampleRate;
    float decay_ = 1.0f;
    float airSpread_ = 0.0f;
    uint64_t sampleIndex_ = 0;
    Random random_;
    CorrelatedColorBank colors_;
};

static constexpr std::size_t kReconstructionPhaseCount = 32;
static constexpr std::size_t kReconstructionTapCount = 128;
static constexpr float kReconstructionCutoff = 0.85f;

// The clap is tuned at 44.1 kHz. This table keeps the shape together at the
// other sample rates without putting an interpolator in each voice
struct ReconstructionTable {
    ReconstructionTable() {
        constexpr int firstOffset = -63;
        constexpr float radius = 64.0f;
        for (std::size_t phase = 0;
             phase <= kReconstructionPhaseCount; ++phase) {
            const float fraction = static_cast<float>(phase)
                / static_cast<float>(kReconstructionPhaseCount);
            float sum = 0.0f;
            for (std::size_t tap = 0; tap < kReconstructionTapCount; ++tap) {
                const float offset = static_cast<float>(
                    firstOffset + static_cast<int>(tap));
                const float distance = fraction - offset;
                const float absolute = std::fabs(distance);
                float value = 0.0f;
                if (absolute < radius) {
                    const float argument = static_cast<float>(kClapPi)
                        * kReconstructionCutoff * distance;
                    const float sinc = std::fabs(argument) < 1.0e-7f
                        ? 1.0f : std::sin(argument) / argument;
                    const float window = 0.42f
                        + 0.50f * std::cos(
                            static_cast<float>(kClapPi) * distance / radius)
                        + 0.08f * std::cos(
                            2.0f * static_cast<float>(kClapPi)
                                * distance / radius);
                    value = kReconstructionCutoff * sinc * window;
                }
                coefficients[phase][tap] = value;
                sum += value;
            }
            const float inverse = 1.0f
                / std::max(std::fabs(sum), 1.0e-12f);
            for (float &value : coefficients[phase]) {
                value *= inverse;
            }
        }
    }

    std::array<std::array<float, kReconstructionTapCount>,
               kReconstructionPhaseCount + 1> coefficients = {{}};
};

static const ReconstructionTable kReconstructionTable;

class CoreUpsampler {
public:
    void configure(float sourceStep) {
        sourceStep_ = clampf(sourceStep, 0.0f, 1.0f);
        sourcePosition_ = 0.0;
        generatedSamples_ = 0;
        history_.fill(CoreFrame());
    }

    CoreFrame process(ClapCore &core) {
        constexpr int firstOffset = -63;
        constexpr int lastOffset = 64;
        const int64_t center = static_cast<int64_t>(
            std::floor(sourcePosition_));
        ensureGenerated(center + lastOffset, core);
        const float fraction = static_cast<float>(
            sourcePosition_ - static_cast<double>(center));
        const float phasePosition = fraction * kReconstructionPhaseCount;
        const std::size_t phase = std::min<std::size_t>(
            static_cast<std::size_t>(std::floor(phasePosition)),
            kReconstructionPhaseCount - 1);
        const float phaseMix = phasePosition - static_cast<float>(phase);
        CoreFrame output;
        for (int offset = firstOffset; offset <= lastOffset; ++offset) {
            const std::size_t tap = static_cast<std::size_t>(
                offset - firstOffset);
            const float first =
                kReconstructionTable.coefficients[phase][tap];
            const float second =
                kReconstructionTable.coefficients[phase + 1][tap];
            const float coefficient = first
                + phaseMix * (second - first);
            const CoreFrame source = sourceAt(center + offset);
            output.full += coefficient * source.full;
            output.dryBody += coefficient * source.dryBody;
        }
        sourcePosition_ += sourceStep_;
        output.full = flushDenormal(output.full);
        output.dryBody = flushDenormal(output.dryBody);
        return output;
    }

private:
    void ensureGenerated(int64_t finalIndex, ClapCore &core) {
        const uint64_t final = finalIndex > 0
            ? static_cast<uint64_t>(finalIndex) : 0u;
        while (generatedSamples_ <= final) {
            history_[generatedSamples_ % history_.size()] = core.process();
            ++generatedSamples_;
        }
    }

    CoreFrame sourceAt(int64_t index) const {
        if (index < 0
            || static_cast<uint64_t>(index) >= generatedSamples_) {
            return CoreFrame();
        }
        return history_[
            static_cast<uint64_t>(index) % history_.size()];
    }

    std::array<CoreFrame, 256> history_ = {{}};
    uint64_t generatedSamples_ = 0;
    double sourcePosition_ = 0.0;
    float sourceStep_ = 1.0f;
};

} // namespace ClapDetail

// Noise at 0.5 is the fitted clap. Turn it left for the hard body, or right
// for more air and a wider, denser tail
class ClapVoice {
public:
    void init(double sampleRate, uint32_t seed = 0x0606C1A9u) {
        sampleRate_ = std::isfinite(sampleRate)
            && sampleRate >= ClapDetail::kMinimumSupportedSampleRate
            && sampleRate <= ClapDetail::kMaximumSupportedSampleRate
            ? sampleRate : ClapDetail::kReferenceSampleRate;
        core_.init(seed);
        stop();
    }

    void trigger(float decayPercent, float pitchRatio,
                 float noiseAmount = 0.5f) {
        const float safeDecay = std::isfinite(decayPercent)
            ? clampf(decayPercent, 0.05f, 1.0f) : 1.0f;
        const float ratio = std::isfinite(pitchRatio)
            ? clampf(std::fabs(pitchRatio), 0.5f, 2.0f) : 1.0f;
        const float normalizedNoise = std::isfinite(noiseAmount)
            ? clampf(noiseAmount, 0.0f, 1.0f) : 0.5f;
        // The left half brings in the colored noise. Above center I leave the
        // attack alone and add brightness and density later in the tail
        noiseLayerGain_ = 2.0f * std::min(normalizedNoise, 0.5f);
        airSpread_ = normalizedNoise > 0.5f
            ? 2.0f * (normalizedNoise - 0.5f) : 0.0f;
        const double desiredCoreRate = ClapDetail::kReferenceSampleRate * ratio;
        const double coreRate = std::min(desiredCoreRate, sampleRate_);
        const float colorSourceStep = static_cast<float>(
            desiredCoreRate / coreRate);
        coreStep_ = static_cast<float>(coreRate / sampleRate_);

        core_.trigger(coreRate, safeDecay, colorSourceStep, airSpread_);
        upsampler_.configure(coreStep_);
        const double airCutoff = std::min(
            ClapDetail::kAirHighpassHz, 0.35 * sampleRate_);
        airHighpassCoefficient_ = static_cast<float>(std::exp(
            -2.0 * ClapDetail::kClapPi * airCutoff / sampleRate_));
        airHighpassInput_ = 0.0f;
        airHighpassOutput_ = 0.0f;
        maximumSamples_ = std::max<uint32_t>(1, static_cast<uint32_t>(
            std::ceil(sampleRate_ * ClapDetail::kVoiceDurationSeconds)));
        sampleIndex_ = 0;
        active_ = true;
    }

    float process() {
        if (!active_) {
            return 0.0f;
        }

        const ClapDetail::CoreFrame raw = coreStep_ == 1.0f
            ? core_.process() : upsampler_.process(core_);
        const double time = static_cast<double>(sampleIndex_) / sampleRate_;
        const double fadePosition = std::max(0.0, std::min(
            (time - 0.290)
                / (ClapDetail::kVoiceDurationSeconds - 0.290), 1.0));
        const double fadeBase = 0.5 + 0.5
            * std::cos(ClapDetail::kClapPi * fadePosition);
        const double fade = fadeBase * fadeBase * fadeBase * fadeBase;
        const double faded = static_cast<double>(raw.full) * fade;
        const double calibrated = ClapDetail::kSaturationThreshold * std::tanh(
            faded / ClapDetail::kSaturationThreshold);
        double saturated = calibrated;
        if (airSpread_ > 0.0f) {
            const double dryFaded = static_cast<double>(raw.dryBody) * fade;
            const double dry = ClapDetail::kSaturationThreshold * std::tanh(
                dryFaded / ClapDetail::kSaturationThreshold);
            const double calibratedNoise = calibrated - dry;
            const float residual = static_cast<float>(calibratedNoise);
            airHighpassOutput_ = flushDenormal(
                airHighpassCoefficient_ * (airHighpassOutput_
                    + residual - airHighpassInput_));
            airHighpassInput_ = flushDenormal(residual);

            // This only pushes the colored part. The first slap stays put
            // while the high noise and compressed wash grow behind it
            const double lateWeight = ClapDetail::raisedCosineRamp(
                time, 0.045, 0.170);
            const double spread = static_cast<double>(airSpread_);
            const double directGain = 1.0 + spread
                * (0.40 + 0.60 * lateWeight);
            const double densityMix = spread
                * (0.20 + 0.55 * lateWeight);
            const double denseNoise = ClapDetail::kAirDensityThreshold
                * std::tanh(2.0 * calibratedNoise
                    / ClapDetail::kAirDensityThreshold);
            const double airMix = spread
                * (0.15 + 0.50 * lateWeight);
            saturated = dry + directGain * calibratedNoise
                + densityMix * denseNoise
                + airMix * static_cast<double>(airHighpassOutput_);
        } else if (noiseLayerGain_ != 1.0f) {
            const double dryFaded = static_cast<double>(raw.dryBody) * fade;
            const double dry = ClapDetail::kSaturationThreshold * std::tanh(
                dryFaded / ClapDetail::kSaturationThreshold);
            const double calibratedNoise = calibrated - dry;
            saturated = dry
                + static_cast<double>(noiseLayerGain_) * calibratedNoise;
        }
        const float output = flushDenormal(static_cast<float>(
            saturated * ClapDetail::kOutputTrim));

        ++sampleIndex_;
        if (sampleIndex_ >= maximumSamples_) {
            active_ = false;
        }
        return output;
    }

    bool isActive() const { return active_; }

    void stop() {
        active_ = false;
        sampleIndex_ = 0;
        maximumSamples_ = 0;
        coreStep_ = 1.0f;
        noiseLayerGain_ = 1.0f;
        airSpread_ = 0.0f;
        airHighpassCoefficient_ = 0.0f;
        airHighpassInput_ = 0.0f;
        airHighpassOutput_ = 0.0f;
        upsampler_.configure(1.0f);
    }

private:
    double sampleRate_ = ClapDetail::kReferenceSampleRate;
    bool active_ = false;
    uint32_t sampleIndex_ = 0;
    uint32_t maximumSamples_ = 0;
    float coreStep_ = 1.0f;
    float noiseLayerGain_ = 1.0f;
    float airSpread_ = 0.0f;
    float airHighpassCoefficient_ = 0.0f;
    float airHighpassInput_ = 0.0f;
    float airHighpassOutput_ = 0.0f;
    ClapDetail::ClapCore core_;
    ClapDetail::CoreUpsampler upsampler_;
};

} // namespace SynthDrums606
