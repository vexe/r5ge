#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Load the contents of the specified string into memory, stripping out C++ comments and extra spaces
//============================================================================================================

void CodeNode::SerializeFrom (const String& source)
{
	uint start(0), end (source.GetSize());

	while (start < end)
	{
		start = mChildren.Expand().SerializeFrom(source, start, end);
	}
}

//============================================================================================================
// Load the specified segment
//============================================================================================================

uint CodeNode::SerializeFrom (const String& source, uint start, uint end)
{
	if (start < end)
	{
		if (end > source.GetSize()) end = source.GetSize();
		while (start < end && source[start] < 33) ++start;

		uint lineStart (start);
		bool prag = (source[start] == '#');

		for (; start <= end; ++start)
		{
			char ch = source[start];

			if ((!prag && ch == ';') || (prag && ch < ' ' && ch != '\t'))
			{
				source.GetTrimmed(mLine, lineStart, start++);
				mLine.TrimCode();
				return start;
			}

			if (ch == '{')
			{
				source.GetTrimmed(mLine, lineStart, start++);
				mLine.TrimCode();

				while (start < end)
				{
					CodeNode& code = mChildren.Expand();
					start = code.SerializeFrom(source, start, end);

					if (code.mLine.IsEmpty())
					{
						mChildren.Shrink();
						break;
					}
				}
				return start;
			}

			if (start < end && source[start] == '}')
			{
				return ++start;
			}
		}
	}
	return end;
}

//============================================================================================================
// Save the structure's contents out into the specified string
//============================================================================================================

void CodeNode::SerializeTo (String& out, uint tabs) const
{
	bool root = mLine.IsEmpty();

	if (!root)
	{
		// Add a new line in front of new scope statements for clarity
		if (mChildren.IsValid() && out.IsValid()) out << "\n";

		bool prag = (mLine[0] == '#');
		if (!prag) for (uint i = 0; i < tabs; ++i) out << "\t";

		out << mLine;
		out << ((prag || mChildren.IsValid()) ? "\n" : ";\n");
	}

	if (mChildren.IsValid())
	{
		if (!root)
		{
			for (uint i = 0; i < tabs; ++i) out << "\t";
			out << "{\n";
		}

		FOREACH(i, mChildren) mChildren[i].SerializeTo(out, root ? 0 : tabs+1);

		if (!root)
		{
			for (uint i = 0; i < tabs; ++i) out << "\t";
			out << "}\n";
		}
	}
}

//============================================================================================================
// Save the structure's contents out into the specified string, using the specified list of #defines
//============================================================================================================

void CodeNode::SerializeTo (String& out, const Array<String>& defines) const
{
	CodeNode copy;
	SerializeTo(copy, defines);
	copy.SerializeTo(out);
}

//============================================================================================================
// Save the structure's contents into the specified node using the specified list of #defines
//============================================================================================================

struct IfDef
{
	bool process;
	bool add;
};

void CodeNode::SerializeTo (CodeNode& out, const Array<String>& defines) const
{
	out.mLine = mLine;

	if (mChildren.IsValid())
	{
		Array<IfDef> block;
		bool save (true);

		FOREACH(i, mChildren)
		{
			const CodeNode& child = mChildren[i];
			const String& line = child.mLine;

			if (line[0] == '#' && !line.BeginsWith("#pragma ", true))
			{
				bool updateBlock (false);

				if (child.mLine.BeginsWith("#endif", true))
				{
					block.Shrink();
					updateBlock = true;
				}
				else if (child.mLine.BeginsWith("#else if ", true))
				{
					bool val (false);

					FOREACH(b, defines)
					{
						uint loc = child.mLine.Find(defines[b]);

						if (loc < child.mLine.GetSize() && loc > 0)
						{
							val = (child.mLine[loc-1] != '!');
							break;
						}
					}

					IfDef& ifdef = block.Back();
					ifdef.add = val && ifdef.process;
					if (val) ifdef.process = false;
					updateBlock = true;
				}
				else if (child.mLine.BeginsWith("#else", true))
				{
					ASSERT(block.IsValid(), "Mismatched '#else'. Did you mean '#else if'?");

					if (block.IsValid())
					{
						IfDef& ifdef = block.Back();
						bool val = ifdef.add;
						ifdef.add = !val && ifdef.process;
						if (val) ifdef.process = false;
						updateBlock = true;
					}
				}
				else if (child.mLine.BeginsWith("#if ", true))
				{
					bool val (false);

					FOREACH(b, defines)
					{
						uint loc = child.mLine.Find(defines[b]);

						if (loc < child.mLine.GetSize() && loc > 0)
						{
							val = (child.mLine[loc-1] != '!');
							break;
						}
					}

					bool process = block.IsEmpty() || block.Back().process;
					IfDef& ifdef = block.Expand();
					ifdef.add = val && process;
					ifdef.process = !val;
					updateBlock = true;
				}

				if (updateBlock)
				{
					save = true;

					FOREACH(b, block)
					{
						if (!block[b].add)
						{
							save = false;
							break;
						}
					}
				}
			}
			else if (save)
			{
				CodeNode& node = out.mChildren.Expand();
				child.SerializeTo(node, defines);
			}
		}
	}
}