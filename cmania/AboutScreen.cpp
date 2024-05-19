#include "AboutScreen.h"
class AboutScreen : public Screen {
	virtual void Render(GameBuffer& buf) {
		buf.DrawString(
			"Cmania\n"
			"�ڿ���̨�ն��ϵ�����\n"
			"\n"
			"�� telecomadm1145 ����\n"
			"ԭʼ��Ϸ: osu (by ppy)\n"
			"��Ϸ����ʹ�� MIT ���֤�ַ�\n"
			"�������������� ���� �޸� �ٷַ�����Ϸ\n"
			"ף����Ϸ���� UwU\n"
			"[Esc] ����\n", 0, 0, {}, {});
	}
	virtual void Key(KeyEventArgs kea) {
		if (kea.Pressed) {
			if (kea.Key == ConsoleKey::Escape) {
				parent->Back();
				return;
			}
		}
	}
};

Screen* MakeAboutScreen() {
	return new AboutScreen();
}
