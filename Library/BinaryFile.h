
//	バイナリーファイル

#pragma once
class BinaryFile
{
protected:

	//	データ
	std::unique_ptr<char[]> m_data;

	//	サイズ
	unsigned int m_size;

public:

	/// <summary>
	/// ファイル名を指定してバイナリデータをメモリにロード
	/// </summary>
	/// <param name="fileName">読み込むファイルのパス</param>
	/// <returns>読み込んだデータを保持するBinaryFileオブジェクト</returns>
	static BinaryFile LoadFile(const wchar_t* fileName);

	/// <summary>
	/// コンストラクタ
	/// </summary>
	BinaryFile();

	/// <summary>
	/// ムーブコンストラクタ
	/// </summary>
	/// <param name="in">ムーブ元のBinaryFileオブジェクト</param>
	BinaryFile(BinaryFile&& in);

	/// <summary>
	/// ロードされたデータの先頭ポインタを取得
	/// </summary>
	/// <returns>データバッファへのポインタ</returns>
	char* GetData() { return m_data.get(); }

	/// <summary>
	/// ロードされたデータのサイズを取得
	/// </summary>
	/// <returns>データのバイトサイズ</returns>
	unsigned int GetSize() const { return m_size; }
};

