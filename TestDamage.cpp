#include "pch.h"

#include <algorithm>

#include <locale.h>
#include <Utility.h>
#include <WindowsUtility.h>

#include "Vector.h"

using namespace std;
//using namespace Core;

static FILE* file = nullptr;

Core::Vector2D WeaponValue() { return Core::Vector2D(204, 270); }
double WeaponMultipler() { return 1.2; }
double WeaponCritical() { return 0.15; }
double BaseCriticalDamageRate() { return 1.25; }
double MotionValue() { return 0.11; }

double PhysicalDefense() { return 0.45; }
double ElementalDefense() { return 0.2; }

//------------------------------------------------------------------------------
int MaxAttackSkillLevel() { return 7; }

pair<int, double> AttackSkillBonus(int level)
{
	int damage[] = { 0, 3, 6, 9, 12, 15, 18, 21, };
	double critical[] = { 0, 0, 0, 0, 0, 0.05, 0.05, 0.05, };
	if (0 <= level && level < COUNT_OF(damage)) 
	{ return make_pair(damage[level], critical[level]); }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxElementalSkillLevel() { return 5; }

pair<int, double> ElementalSkillLevel(int level)
{
	int damage[] = { 0, 30, 60, 100, 100, 100, };
	double modifier[] = { 0, 0, 0, 0, 0.05, 0.1, };
	if (0 <= level && level < COUNT_OF(damage))
	{ return make_pair(damage[level], modifier[level]); }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxCriticalEyeSkillLevel() { return 7; }

double CriticalEye(int level)
{
	double modifier[] = { 0, 0.03, 0.06, 0.1, 0.15, 0.2, 0.25, 0.3 };
	if (0 <= level && level < COUNT_OF(modifier)) { return modifier[level]; }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxSuperCriticalSkillLevel() { return 3; }

double SuperCritical(int level)
{
	double modifier[] = { 0, 0.05, 0.1, 0.15, };
	if (0 <= level && level < COUNT_OF(modifier))
	{ return modifier[level]; }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
int MaxExploitWeaknessSkillLevel() { return 3; }

double ExploitWeakness(int level)
{
	double modifier[] = { 0, 0.15, 0.3, 0.5, };
	if (0 <= level && level < COUNT_OF(modifier))
	{ return modifier[level]; }
	throw invalid_argument("");
}

//------------------------------------------------------------------------------
double FireDragonsGambit() { return 0.1; }

//------------------------------------------------------------------------------
struct Desc
{
	pair<int, double> attackBonus;
	pair<int, double> elementalBonus;
	double criticalEye;
	double superCritical;
	double exploitWeakness;
	double fireDragonGambit;
};

//------------------------------------------------------------------------------
void Calculate(const Desc& desc)
{
	// 무기 기본 대미지
	Core::Vector2D rawDamage = WeaponValue();
	rawDamage.x /= WeaponMultipler();
	rawDamage.y /= 10;

	// 대미지 보너스
	Core::Vector2D rawDamageWithBonus =
		rawDamage +
		Core::Vector2D(
			desc.attackBonus.first, 
			desc.elementalBonus.first);

	// 기본 대미지 * 강격병 * 거리 크리티컬
	Core::Vector2D modifiedBaseDamage(
		rawDamageWithBonus.x * 1.5 * 1.5,
		rawDamageWithBonus.y * 1.5 * 1.5 * (1 + desc.elementalBonus.second));

	// 회심률
	auto criticalProbability =
		WeaponCritical() +
		desc.attackBonus.second +
		desc.criticalEye +
		desc.exploitWeakness;

	if (criticalProbability > 1) { criticalProbability = 1; }

	// 물리 회심 배율
	auto physicalCriticalRate =
		BaseCriticalDamageRate() +
		desc.superCritical;

	// 속성 회심 배율
	auto elementalCriticalRate =
		1 +
		desc.fireDragonGambit;

	// 기대 대미지
	Core::Vector2D criticalExpectedDamage(
		modifiedBaseDamage.x * criticalProbability * physicalCriticalRate +
		modifiedBaseDamage.x * (1 - criticalProbability),
		modifiedBaseDamage.y * criticalProbability * elementalCriticalRate +
		modifiedBaseDamage.y * (1 - criticalProbability)
	);

	// 모션 적용
	Core::Vector2D motionDamage = criticalExpectedDamage * MotionValue();

	// 방어력 적용
	Core::Vector2D finalDamage =
		motionDamage *
		Core::Vector2D(PhysicalDefense(), ElementalDefense());

	fprintf(
		file,
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t%.3f\t"
		"%.3f\t"
		"%d"
		"\n",
		rawDamageWithBonus.x, rawDamageWithBonus.y,
		modifiedBaseDamage.x, modifiedBaseDamage.y,
		criticalProbability, physicalCriticalRate, elementalCriticalRate,
		criticalExpectedDamage.x, criticalExpectedDamage.y,
		motionDamage.x, motionDamage.y,
		finalDamage.x, finalDamage.y,
		finalDamage.x + finalDamage.y, 
		(int) (finalDamage.x + finalDamage.y));
}

//------------------------------------------------------------------------------
vector<wstring> Tokenize(const wstring& s_)
{
	vector<wstring> result;

	wstring s = s_;

	size_t pos = 0;
	wstring token;
	while ((pos = s.find(L',')) != wstring::npos) 
	{
		token = s.substr(0, pos);
		result.push_back(token);
		s.erase(0, pos + 1);
	}
	result.push_back(s);

	return result;
}

//------------------------------------------------------------------------------
struct Set
{
	wstring name;
	int rarity;
	int defense;
	wstring skill;
};

struct Armor
{
	enum PartType
	{
		Head,
		Body,
		Arm,
		Waist,
		Leg,

		Count,
	};

	Set*			set;
	PartType		type;
	wstring			name;
	vector<pair<wstring, int>>	skills;
	vector<int>		slots;
};

static wstring g_skills[] =
{
	L"공격",
	L"간파",
	L"슈퍼 회심",
	L"약점 특효",
	L"번개속성 공격 강화",
	L"화룡의 비기",

	// 활 강화
	L"통상탄/통상화살 강화",
	L"산탄/강사 강화",
	L"활 모으기 단계 해제",

	// 유틸리티
	L"체술",
};

struct Decorator
{
	wstring		name;
	wstring		skill;
	int			skillIndex;
	int			rarity;
	int			slotSize;
};

struct ArmorInstance
{
	Armor* original = nullptr;
	Decorator* decorators[3];

	ArmorInstance()
	{
		memset(decorators, 0, sizeof(Decorator*) * COUNT_OF(decorators));
	}
};

struct GeneralizedArmor
{
	int* skills;
	int skillCount;
	list<ArmorInstance*>	source;

	GeneralizedArmor(int skillCount)
		: skillCount(skillCount)
	{
		skills = new int[skillCount] { 0 };
	}

	~GeneralizedArmor()
	{
		delete[] skills;
	}

	GeneralizedArmor& operator = (GeneralizedArmor& rhs)
	{
		delete[] skills;

		skillCount = rhs.skillCount;
		skills = new int[skillCount];

		memcpy(skills, rhs.skills, sizeof(int) * skillCount);

		source = rhs.source;

		return *this;
	}

	bool operator == (const GeneralizedArmor& rhs)
	{
		for (int i = 0; i < skillCount; ++i)
		{
			if (skills[i] != rhs.skills[i])
			{
				return false;
			}
		}

		return true;
	}

	bool operator <= (const GeneralizedArmor& rhs)
	{
		for (int i = 0; i < skillCount; ++i)
		{
			if (skills[i] > rhs.skills[i])
			{
				return false;
			}
		}

		return true;
	}
};

static vector<Set*> g_sets;

static map<Armor::PartType, vector<Armor*>*> g_armors;

static map<Armor::PartType, vector<GeneralizedArmor*>*> g_generalized;

static vector<Decorator*> g_decorators;

//------------------------------------------------------------------------------
void LoadArmors()
{
	fopen_s(&file, "ArmorData", "r,ccs=UTF-8");

	Set* curSet = nullptr;
	int offset = 0;

	g_armors.insert(make_pair(Armor::Head, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Body, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Arm, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Waist, new vector<Armor*>));
	g_armors.insert(make_pair(Armor::Leg, new vector<Armor*>));

	wchar_t buffer[1024];
	while (fgetws(&buffer[0], 1024, file) != nullptr)
	{
		// 먼저 뉴라인 문자를 제거한다
		wstring str = buffer;
		size_t pos;
		if ((pos = str.find(L'\n')) != wstring::npos)
		{ 
			str = str.substr(0, pos);
		}

		// 쉼표로 자른다
		auto tokens = Tokenize(str);

		if (offset == 0)
		{
			curSet = new Set;
			curSet->name = tokens[0];
			curSet->rarity = _wtoi(tokens[1].c_str());
			curSet->defense = _wtoi(tokens[2].c_str());
			if (tokens.size() >= 7) { curSet->skill = tokens[6].c_str(); }
			g_sets.push_back(curSet);
		}
		else if (offset < 6)
		{
			if (tokens.size() >= 3)
			{
				auto part = new Armor;
				part->type = (Armor::PartType) (offset - 1);
				part->set = curSet;
				part->name = tokens[0];

				int skillCount = _wtoi(tokens[1].c_str());
				for (int i = 0; i < skillCount; ++i)
				{
					part->skills.push_back(make_pair(
						tokens[2 + i * 2 + 0],
						_wtoi(tokens[2 + i * 2 + 1].c_str())));
				}

				int slotCount = _wtoi(tokens[2 + skillCount * 2].c_str());
				for (int i = 0; i < slotCount; ++i)
				{
					part->slots.push_back(
						_wtoi(tokens[3 + skillCount * 2 + i].c_str()));
				}

				g_armors[part->type]->push_back(part);

#if 0
				WindowsUtility::Debug(L"\"%s\"\t", part->name.c_str());
				for (int i = 0; i < part->skills.size(); ++i)
				{
					WindowsUtility::Debug(
						L"%s Lv.%d\t", 
						part->skills[i].first.c_str(),
						part->skills[i].second);
				}
				if (!part->set->skill.empty())
				{
					WindowsUtility::Debug(
						L"%s\t",
						part->set->skill.c_str());
				}
				for (int i = 0; i < part->slots.size(); ++i)
				{
					WindowsUtility::Debug(L"[%d] ", part->slots[i]);
				}
				WindowsUtility::Debug(L"\n");
#endif
			}
		}

		offset = (offset + 1) % 7;
	}

	fclose(file);
}

//------------------------------------------------------------------------------
int GetSkillIndex(const wstring& skill)
{
	for (int i = 0; i < COUNT_OF(g_skills); ++i)
	{
		if (g_skills[i] == skill)
		{
			return i;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
void LoadDecorators()
{
	fopen_s(&file, "DecorationData", "r,ccs=UTF-8");

	wchar_t buffer[1024];
	while (fgetws(&buffer[0], 1024, file) != nullptr)
	{
		// 먼저 뉴라인 문자를 제거한다
		wstring str = buffer;
		size_t pos;
		if ((pos = str.find(L'\n')) != wstring::npos)
		{
			str = str.substr(0, pos);
		}

		// 쉼표로 자른다
		auto tokens = Tokenize(str);

		if (tokens.size() >= 4)
		{
			int index = GetSkillIndex(tokens[0]);
			if (index >= 0)
			{
				auto dec = new Decorator;
				dec->name = tokens[3];
				dec->skill = tokens[0];
				dec->skillIndex = index;
				dec->rarity = _wtoi(tokens[2].c_str());
				dec->slotSize = _wtoi(tokens[1].c_str());

				g_decorators.push_back(dec);
			}

			//WindowsUtility::Debug(L"%s %s R%d [%d]\n", dec->name.c_str(), dec->skill.c_str(), dec->rarity, dec->slotSize);
		}
	}

	fclose(file);
}

//------------------------------------------------------------------------------
void FilterArmors()
{
	g_generalized.insert(make_pair(Armor::Head, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Body, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Arm, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Waist, new vector<GeneralizedArmor*>));
	g_generalized.insert(make_pair(Armor::Leg, new vector<GeneralizedArmor*>));

	int total = 1;

	for (int i = 0; i < Armor::Count; ++i)
	{
		auto parts = g_armors[(Armor::PartType) i];
		for (int j = 0; j < parts->size(); ++j)
		{
			auto part = (*parts)[j];

			GeneralizedArmor general(COUNT_OF(g_skills));

			// 셋트 스킬이 포함시킬 스킬인 경우
			int index = GetSkillIndex(part->set->skill);
			if (index >= 0) { general.skills[index]++; }

			// 개별 파트 스킬이 포함시킬 스킬인 경우
			for (int k = 0; k < part->skills.size(); ++k)
			{
				int index = GetSkillIndex(part->skills[k].first);
				if (index >= 0)
				{
					general.skills[index] += part->skills[k].second;
				}
			}

			// 장식주를 처리해보자!
			ArmorInstance inst;
			inst.original = part;

			function<void(int)> SelectDecorator;
			SelectDecorator = [&](int slot)
			{
				// 슬롯을 더 끼울 수 있으면
				if (slot < part->slots.size())
				{
					// 장식주를 모두 순회하면서
					for (int d = 0; d < g_decorators.size(); ++d)
					{
						auto dec = g_decorators[d];

						// 끼울 수 있는 장식주를 끼우고 
						if (part->slots[slot] >= dec->slotSize)
						{
							int preserved = general.skills[dec->skillIndex];

							inst.decorators[slot] = dec;

							general.skills[dec->skillIndex]++;

							// 다음 슬롯으로 넘어간다
							SelectDecorator(slot + 1);

							general.skills[dec->skillIndex] = preserved;
						}
					}
				}
				else
				{
					auto newInst = new ArmorInstance;
					*newInst = inst;

					// scan
					bool duplication = false;
					for (int k = 0; k < g_generalized[(Armor::PartType) i]->size(); ++k)
					{
						auto prev = (*g_generalized[(Armor::PartType) i])[k];

						if (*prev == general)
						{
							prev->source.push_back(newInst);
							duplication = true;
							break;
						}
					}

					bool added = false;
					if (!duplication)
					{
						auto newGeneral = new GeneralizedArmor(COUNT_OF(g_skills));
						*newGeneral = general;

						newGeneral->source.push_back(newInst);

						g_generalized[(Armor::PartType) i]->push_back(newGeneral);

						added = true;
					}
				}
			};

			SelectDecorator(0);
		}

		for (int j = 0; j < g_generalized[(Armor::PartType) i]->size(); )
		{
			// 다른 조합보다 불리한 조합은 모두 제거한다
			auto* cur = (*g_generalized[(Armor::PartType) i])[j];

			bool bad = false;

			for (int k = j + 1; k < g_generalized[(Armor::PartType) i]->size(); ++k)
			{
				auto* rhs = (*g_generalized[(Armor::PartType) i])[k];

				if (*cur <= *rhs)
				{
					bad = true;
					break;
				}
			}

			if (bad)
			{
				g_generalized[(Armor::PartType) i]->erase(
					g_generalized[(Armor::PartType) i]->begin() + j);
				delete cur;
			}
			else
			{
				++j;
			}
		}

		for (int j = 0; j < g_generalized[(Armor::PartType) i]->size(); j++)
		{
			auto* general = (*g_generalized[(Armor::PartType) i])[j];

			for (int i = 0; i < COUNT_OF(g_skills); ++i)
			{
				if (general->skills[i] > 0)
				{
					WindowsUtility::Debug(
						L"%s Lv.%d\t",
						g_skills[i].c_str(),
						general->skills[i]);
				}
			}

			WindowsUtility::Debug(L"\n");
		}

		int validParts = g_generalized[(Armor::PartType) i]->size();
		WindowsUtility::Debug(L"%d-------------------------\n", validParts);
		total *= validParts;
	}

	WindowsUtility::Debug(L"Total Combination = %d\n", total);
}

//------------------------------------------------------------------------------
template <class T>
inline void hash_combine(std::size_t & s, const T & v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

struct Combination
{
	int* skills;
	int skillCount;
	GeneralizedArmor* source[5];

	Combination(int skillCount)
		: skillCount(skillCount)
	{
		skills = new int[skillCount] { 0 };

		memset(source, 0, sizeof(GeneralizedArmor*) * COUNT_OF(source));
	}

	~Combination()
	{
		delete[] skills;
	}

	Combination& operator = (Combination& rhs)
	{
		delete[] skills;

		skillCount = rhs.skillCount;
		skills = new int[skillCount];

		memcpy(skills, rhs.skills, sizeof(int) * skillCount);

		memcpy(source, rhs.source, sizeof(GeneralizedArmor*) * COUNT_OF(source));

		return *this;
	}

	bool operator == (const Combination& rhs)
	{
		for (int i = 0; i < skillCount; ++i)
		{
			if (skills[i] != rhs.skills[i])
			{
				return false;
			}
		}

		return true;
	}

	bool operator <= (const Combination& rhs)
	{
		for (int i = 0; i < skillCount; ++i)
		{
			if (skills[i] > rhs.skills[i])
			{
				return false;
			}
		}

		return true;
	}

	void Combine(GeneralizedArmor* armor, int channel)
	{
		source[channel] = armor;

		for (int i = 0; i < skillCount; ++i)
		{
			skills[i] += armor->skills[i];
		}
	}
};

list<Combination*> g_all;

//------------------------------------------------------------------------------
void PopulateArmors()
{
	// 초기 리스트를 만든다
	list<Combination*> all;

	{
		int channel = 0;
		auto& gs = *g_generalized[(Armor::PartType) channel];

		for (int i = 0; i < gs.size(); ++i)
		{
			auto newComb = new Combination(COUNT_OF(g_skills));
			newComb->Combine(gs[i], channel);
			all.push_back(newComb);
		}

		WindowsUtility::Debug(L"%d\n", all.size());
	}

	// 다음 채널을 추가해서 늘린다
	for (int channel = 1; channel < Armor::Count; ++channel)
	{
		list<Combination*> next;

		auto& gs = *g_generalized[(Armor::PartType) channel];

		// 모든 조합 x 조합의 페어를 일단 생성
		for (auto it = all.begin(); it != all.end(); ++it)
		{
			for (int i = 0; i < gs.size(); ++i)
			{
				auto newComb = new Combination(COUNT_OF(g_skills));
				*newComb = **it;
				newComb->Combine(gs[i], channel);
				next.push_back(newComb);
			}
		}

		// 생성된 페어를 평가한다
		int rejected = 0;
		for (auto it = next.begin(); it != next.end(); )
		{
			auto toEvaluate = *it;

			bool bad = false;

			auto jt = it;
			jt++;

			for (; jt != next.end(); ++jt)
			{
				auto target = *jt;
				if (*toEvaluate <= *target)
				{
					bad = true;
					break;
				}
			}

			if (bad)
			{
				++rejected;
				delete toEvaluate;
				it = next.erase(it);
			}
			else
			{
#if 1
				for (int i = 0; i < COUNT_OF(g_skills); ++i)
				{
					if (toEvaluate->skills[i] > 0)
					{
						WindowsUtility::Debug(
							L"%d\t",
							toEvaluate->skills[i]);
					}
					else
					{
						WindowsUtility::Debug(L"\t");
					}
				}

				WindowsUtility::Debug(L"\n");
#endif

				it++;
			}
		}

		for (auto it = all.begin(); it != all.end(); ++it)
		{
			delete *it;
		}

		all.swap(next);

		WindowsUtility::Debug(L"%d,%d---------------------------------------\n", all.size(), rejected);
	}
}

//------------------------------------------------------------------------------
void TestDamage()
{
	LoadArmors();
	LoadDecorators();
	
	FilterArmors();

	PopulateArmors();

	fopen_s(&file, "log_damage.txt", "w");

	Desc desc;

	for (int a = 0; a <= MaxAttackSkillLevel(); ++a)
	{
		desc.attackBonus = AttackSkillBonus(a);

		for (int b = 0; b <= MaxElementalSkillLevel(); ++b)
		{
			desc.elementalBonus = ElementalSkillLevel(b);

			for (int c = 0; c <= MaxCriticalEyeSkillLevel(); ++c)
			{
				desc.criticalEye = CriticalEye(c);

				for (int d = 0; d <= MaxSuperCriticalSkillLevel(); ++d)
				{
					desc.superCritical = SuperCritical(d);

					for (int e = 0; e <= MaxExploitWeaknessSkillLevel(); ++e)
					{
						desc.exploitWeakness = ExploitWeakness(e);

						for (int f = 0; f < 2; ++f)
						{
							desc.fireDragonGambit = f == 0 ? 0 : FireDragonsGambit();

							fprintf(
								file,
								"%d\t%d\t%d\t%d\t%d\t%d\t",
								a, b, c, d, e, f);

							Calculate(desc);
						}
					}
				}
			}

		}
	}

	fclose(file);
}