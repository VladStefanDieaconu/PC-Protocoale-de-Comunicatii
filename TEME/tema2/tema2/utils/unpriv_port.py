MIN_VALUE = 1024
MAX_VALUE = 65535


def unprivileged_port_type(value, min_value=1024, max_value=MAX_VALUE):
    try:
        int_value = int(value)
    except ValueError:
        raise ValueError('The port should be an integer.')

    if (int_value >= min_value) and (int_value <= max_value):
        return int_value
    else:
        raise ValueError(
            'The port should be between {min_value} and {max_value}'.format(min_value=min_value, max_value=max_value))


def get_unprivileged_port_meta(var_name: str, min_value=MIN_VALUE, max_value=MAX_VALUE):
    return '({min_value}-{max_value})'.format(min_value=min_value,
                                              max_value=max_value)
