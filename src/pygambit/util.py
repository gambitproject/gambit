import contextlib
import pathlib
import tempfile
import typing


@contextlib.contextmanager
def make_temporary(content: typing.Optional[str] = None) -> pathlib.Path:
    """Context manager to create a temporary file containing `content', and
    provide the path to the temporary file.

    If `content' is none, the temporary file is created and then deleted, while
    returning the filename, for another process then to write to that file
    (under the assumption that it is extremely unlikely that another program
    will try to write to that same tempfile name).
    """
    with tempfile.NamedTemporaryFile("w", delete=(content is None)) as f:
        if content:
            f.write(content)
        filepath = pathlib.Path(f.name)
    try:
        yield filepath
    finally:
        filepath.unlink(missing_ok=True)


class MultiIndexSeriesFormatter:
    def __init__(self, max_total_width=120, max_column_width=30):
        self.max_total_width = max_total_width
        self.max_column_width = max_column_width

    def _truncate_text(self, text, max_width):
        """
        Truncate text to specified maximum width using ellipsis.

        Args:
            text (str): Text to truncate
            max_width (int): Maximum allowed width

        Returns:
            str: Truncated text
        """
        # Convert to string and remove any newlines
        text_str = str(text).replace("\n", " ")

        # Leave the text as is if its length is under max_width
        if len(text_str) <= max_width:
            return text_str

        # If max_width is less than 3, just return first characters
        if max_width < 3:
            return text_str[:max_width]

        # Truncate with ellipsis
        return text_str[:max_width-3] + "..."

    def format(self, series: dict[tuple[str, ...], float]):
        """
        Generate a string representation similar to pandas Series with multi-index.

        Args:
            max_total_width (int): Maximum total width of the output
            max_column_width (int): Maximum width for each column

        Returns:
            str: Formatted string representation of the instance
        """

        # Prepare the output lines
        output_lines = []

        # Calculate column widths
        # First, get the maximum width for each column
        column_widths = [0] * len(next(iter(series.keys())))
        value_width = 0

        # Determine column widths for index columns
        for key in series:
            for i, part in enumerate(key):
                column_widths[i] = max(column_widths[i], len(str(part)))

        # Determine value width
        value_width = max(len(str(val)) for val in series.values())

        # Adjust column widths if total exceeds max_total_width
        total_width = sum(column_widths) + value_width + len(next(iter(series.keys())))
        if total_width > self.max_total_width:
            # Proportionally reduce column widths
            reduction_factor = self.max_total_width / total_width
            column_widths = [max(3, int(w * reduction_factor)) for w in column_widths]

        # Limit each column to max_column_width
        column_widths = [min(w, self.max_column_width) for w in column_widths]

        # Format each line
        keys = list(series.keys())
        for i, key in enumerate(keys):
            # Truncate each part of the multi-index
            formatted_parts = [
                self._truncate_text(str(part), width)
                if i == 0 or part != previous_part else self._truncate_text(" " * width, width)
                for part, width, previous_part in zip(key, column_widths, keys[i-1])
            ]

            # Pad or truncate each part to its designated width
            formatted_index = " ".join(
                part.ljust(width) if len(part) <= width else part[:width]
                for part, width in zip(formatted_parts, column_widths)
            )

            # Format the value
            value = self._truncate_text(str(series[key]), self.max_column_width)

            # Combine index and value
            output_lines.append(f"{formatted_index} {value}")

        # Construct the final representation
        repr_str = "\n".join(output_lines)

        return repr_str
